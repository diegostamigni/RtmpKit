#include "../../private/jni/utils.h"
#include "../../private/logger.h"
#include "../../private/mediautils.h"
#include "../../private/rtmp/session/rtmp_watcher_session.h"

bool isRunning, isActive = false;
std::string currentHost, currentPort, currentAppName, currentStreamURL;

jobject getLiveSessionListener(JavaVM *jvm, JNIEnv *env, jobject sender)
{
	if (sender == nullptr || env == nullptr)
	{
		return nullptr;
	}

	const auto senderClass = env->GetObjectClass(sender);
	const auto fieldID = env->GetFieldID(senderClass, "watcherLiveSessionListener",
	                                     "Ltv/diegostamigni/rtmpkit/WatcherLiveSessionListener;");

	return env->GetObjectField(sender, fieldID);
}

void markRunning()
{
	isRunning = true;
}

void markStopped()
{
	isRunning = false;
}

// PIMPL
struct RtmpWatcherDelegateCollector : RtmpKit::RtmpWatcherSessionDelegate
{
	explicit RtmpWatcherDelegateCollector(JNIEnv* env, jobject sender)
		: jvm_(nullptr)
		, env_(nullptr)
		, sender_(env->NewGlobalRef(sender))
		, audioTimebase_(0)
		, videoTimebase_(0)
	{
		const auto rs = env->GetJavaVM(&jvm_);
		assert (rs == JNI_OK);
	}

	virtual ~RtmpWatcherDelegateCollector()
	{
		if (this->env_ == nullptr)
		{
			return;
		}

		if (this->sender_ != nullptr)
		{
			this->env_->DeleteGlobalRef(this->sender_);
			this->sender_ = nullptr;
		}

		if (this->jvm_ != nullptr)
		{
			this->jvm_->DetachCurrentThread();
			this->jvm_->DestroyJavaVM();
			this->jvm_ = nullptr;
		}

		this->env_ = nullptr;
	}

	virtual void handshakeDone() override
	{
	}

	virtual void socketWillConnect() override
	{
	}

	virtual void socketDidConnect() override
	{
	}

	virtual void socketWillClose() override
	{
	}

	virtual void socketDidClose() override
	{
		this->setActive(false);
	}

	virtual void socketFailedToConnect(const std::string &errMessage) override
	{
	}

	virtual void sessionStarted() override
	{
		this->setActive(true);
	}

	virtual void reconnectionRequired() override
	{
	}

	virtual void metadataPacketReceived(RtmpKit::RtmpMetadataMessage&& packet) override
	{
		notifyMetadataPacketReceived(std::forward<RtmpKit::RtmpMetadataMessage>(packet));
	}

	virtual void videoPacketReceived(RtmpKit::RtmpVideoMessage&& packet) override
	{
		notifyVideoPacketReceivedPacketReceived(std::forward<RtmpKit::RtmpVideoMessage>(packet));
	}

	virtual void audioPacketReceived(RtmpKit::RtmpAudioMessage&& packet) override
	{
		notifyAudioPacketReceivedPacketReceived(std::forward<RtmpKit::RtmpAudioMessage>(packet));
	}

	virtual void networkPerformanceMetricsChanged(RtmpKit::NetworkType type,
												  std::size_t preferredByterate) override
	{
	}

private:
	void setActive(bool value)
	{
		isActive = value;

		if (this->jvm_ == nullptr || this->sender_ == nullptr) return;
		updateEnv();

		if (const auto object = getLiveSessionListener(this->jvm_, this->env_, this->sender_))
		{
			callVoidMethod(this->env_, object, value ? "sessionStarted" : "sessionEnded");
		}
	}

	void updateEnv()
	{
		if (this->env_ != nullptr)
		{
			this->jvm_->DetachCurrentThread();
		}

		this->env_ = getJNIEnv(this->jvm_);
	}

	void notifyMetadataPacketReceived(RtmpKit::RtmpMetadataMessage&& packet)
	{
		if (const auto settingsClass = getRtmpSettingsClass())
		{
			updateEnv();

			const auto constructor = this->env_->GetMethodID(settingsClass, "<init>", "(IIIILjava/lang/String;)V");
			const auto object = this->env_->NewObject(settingsClass, constructor,
				static_cast<jint>(packet.height()),
				static_cast<jint>(packet.width()),
				static_cast<jint>(packet.frameRate()),
				static_cast<jint>(packet.audioChannels()),
				this->env_->NewStringUTF(packet.videoCodecId().c_str()));

			if (const auto clazz = this->env_->GetObjectClass(this->sender_))
			{
				if (const auto methodID = this->env_->GetMethodID(clazz, "metadataPacketReceived",
				                                                  "(Ltv/diegostamigni/rtmpkit/rtmp/RtmpSettings;)V"))
				{
					this->env_->CallVoidMethod(this->sender_, methodID, object);
				}
			}
		}
	}

	void notifyVideoPacketReceivedPacketReceived(RtmpKit::RtmpVideoMessage&& packet)
	{
		if (!packet.hasExtendedTimestamp())
		{
			videoTimebase_ = packet.chunkStreamType() == RtmpKit::RtmpChunkStreamType::Type0
			           ? packet.timestamp()
			           : videoTimebase_ + packet.timestamp();
		}

		const auto &payload = packet.data();
		if (payload.empty()) return;

		auto begin = payload.cbegin(), end = payload.cend();
		const auto descriptor = RtmpKit::MediaUtils::unpackVideoData(begin, end, videoTimebase_, true);
		const auto hasDescriptor = descriptor.isValid();

		updateEnv();
		const auto clazz = this->env_->GetObjectClass(this->sender_);
		if (clazz == nullptr) return;

		const auto videoFormatPacketReceived = this->env_->GetMethodID(clazz, "videoFormatPacketReceived", "(JI[B[B)V");
		const auto videoPacketReceived = this->env_->GetMethodID(clazz, "videoPacketReceived", "(J[B)V");

		if (hasDescriptor)
		{
			const auto &sps = descriptor.sps(), &pps = descriptor.pps();

			auto spsByteArray = this->env_->NewByteArray(sps.size());
			this->env_->SetByteArrayRegion(spsByteArray, 0, sps.size(), (const jbyte *) sps.data());

			auto ppsByteArray = this->env_->NewByteArray(pps.size());
			this->env_->SetByteArrayRegion(ppsByteArray, 0, pps.size(), (const jbyte *) pps.data());

			this->env_->CallVoidMethod(this->sender_,
			                           videoFormatPacketReceived,
			                           static_cast<jlong>(videoTimebase_ * 1000),
			                           static_cast<jint>(descriptor.nalUnitSize()),
			                           spsByteArray,
			                           ppsByteArray);
		}

		const auto remainingSize = std::distance(begin, end);
		if (remainingSize < 1) return;

		const auto offset = 5; // ???
		auto videoData = RtmpKit::v8{};
		videoData.reserve(static_cast<std::size_t>(remainingSize) - offset);
		std::move(begin + offset, end, std::back_inserter(videoData));

		auto byteArray = this->env_->NewByteArray(videoData.size());
		this->env_->SetByteArrayRegion(byteArray, 0, videoData.size(), (const jbyte *) videoData.data());

//		const auto data = (const jbyte *) payload.data();
//		const auto index = std::distance(payload.cbegin(), begin) - 1;
//		auto byteArray = this->env_->NewByteArray(remainingSize);
//		this->env_->SetByteArrayRegion(byteArray, index, remainingSize, data);

		this->env_->CallVoidMethod(this->sender_,
								   videoPacketReceived,
								   static_cast<jlong>(videoTimebase_ * 1000),
								   byteArray);
	}

	void notifyAudioPacketReceivedPacketReceived(RtmpKit::RtmpAudioMessage&& packet)
	{
		if (!packet.hasExtendedTimestamp())
		{
			audioTimebase_ = packet.chunkStreamType() == RtmpKit::RtmpChunkStreamType::Type0
			           ? packet.timestamp()
			           : audioTimebase_ + packet.timestamp();
		}

		const auto &payload = packet.data();
		if (payload.empty()) return;

		updateEnv();
		if (auto byteArray = this->env_->NewByteArray(payload.size()))
		{
			const auto data = (const jbyte *) payload.data();
			this->env_->SetByteArrayRegion(byteArray, 0, payload.size(), data);

			if (const auto clazz = this->env_->GetObjectClass(this->sender_))
			{
				if (const auto methodID = this->env_->GetMethodID(clazz, "audioPacketReceived", "(J[B)V"))
				{
					this->env_->CallVoidMethod(this->sender_,
					                           methodID,
					                           static_cast<jlong>(audioTimebase_),
					                           byteArray);
				}
			}
		}
	}

private:
	JavaVM* jvm_;
	JNIEnv* env_;
	jobject sender_;
	RtmpKit::u32 audioTimebase_;
	RtmpKit::u32 videoTimebase_;
};

std::shared_ptr<RtmpWatcherDelegateCollector> collector;
std::shared_ptr<RtmpKit::RtmpWatcherSession> rtmpSession;

JNI_METHOD_RET(void)
Java_com_diegostamigni_rtmpkit_rtmp_RtmpWatcherView_start(JNIEnv* env, jobject sender,
                                               jstring host,
                                               jstring port,
                                               jstring appName,
                                               jstring streamURL)
{
    if (rtmpSession == nullptr)
    {
        rtmpSession = std::make_shared<RtmpKit::RtmpWatcherSession>();
		collector = std::make_shared<RtmpWatcherDelegateCollector>(env, sender);
		rtmpSession->addWatcherDelegate(collector);
    }

	if (env->GetStringUTFLength(host) <= 0
		|| env->GetStringUTFLength(port) <= 0
		|| env->GetStringUTFLength(appName) <= 0
		|| env->GetStringUTFLength(streamURL) <= 0
		|| isRunning)
	{
		return;
	}

	// TODO(diegostamigni): the param `isCopy' determines whenever the str needs to be released using ReleaseStringUTFChars(...)
	currentHost = std::string(env->GetStringUTFChars(host, nullptr));
	currentPort = std::string(env->GetStringUTFChars(port, nullptr));
	currentAppName = std::string(env->GetStringUTFChars(appName, nullptr));
	currentStreamURL = std::string(env->GetStringUTFChars(streamURL, nullptr));

	rtmpSession->start(currentHost, currentPort, currentAppName, currentStreamURL);
	markRunning();
}

JNI_METHOD_RET(void)
Java_com_diegostamigni_rtmpkit_rtmp_RtmpWatcherView_end(JNIEnv* env, jobject sender)
{
	if (rtmpSession == nullptr || !isRunning)
	{
		return;
	}

	rtmpSession->end();
	markStopped();
}

JNI_METHOD_RET(jboolean)
Java_com_diegostamigni_rtmpkit_rtmp_RtmpWatcherView_isConnected(JNIEnv* env, jobject sender)
{
	return (jboolean) (isRunning && isActive);
}
