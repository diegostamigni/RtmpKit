#include "../../private/flv/flv_amf_mediatag.h"
#include "../../private/amf/amf.h"
using namespace RtmpKit;

FLVMediaAMFTag::FLVMediaAMFTag()
	: FLVMediaTag()
	, width_{ 1280. }
	, height_{ 720. }
	, videoframerate_ { 0. }
	, videodatarate_{ 850. }
	, framerate_{ 30. }
	, videocodecidStr_ { "avc" }
	, audiodatarate_{ 60. }
	, audiosamplerate_{ 48000. }
	, audiochannels_{ 1 }
	, audiocodecidStr_ { "aac" }
	, avcLevel_ { 41. }
	, avcProfile_ { 100. }
{
}

void FLVMediaAMFTag::deserialize()
{
	// TODO(diegostamigni): the deserializer version should be extracted from the first packet (we're always using AMF v0)
	auto deserializer = AMF::v0::Deserializer{};
	auto result = deserializer.deserialize(this->payload_);

	for (const auto& variant : result)
	{
		const auto* onMetaData = boost::get<AMF::StringVariant>(&variant);
		if (onMetaData && onMetaData->value() == FLV_AMF_ONMETADATA) continue;

		if (const auto* properties = boost::get<AMF::EcmaArrayVariant>(&variant))
		{
			auto propDict = properties->value();
			deserializeFromDict(propDict);
			continue;
		}

		if (const auto* properties = boost::get<AMF::ObjectVariant>(&variant))
		{
			auto propDict = properties->value();
			deserializeFromDict(propDict);
			continue;
		}
	}
}

void FLVMediaAMFTag::deserializeFromDict(const AMF::TDictSubset& propDict)
{
	// TODO(diegostamigni): move into the if block [aka structured-bindings] with c++17
	auto duration = propDict.find(FLV_AMF_PROP_DURATION);
	if (duration != propDict.end())
	{
		if (const auto* item = boost::get<AMF::NumberVariant>(&duration->second))
		{
			this->setDuration(item->value());
		}
	}

	auto height = propDict.find(FLV_AMF_PROP_HEIGHT);
	if (height != propDict.end())
	{
		if (const auto* item = boost::get<AMF::NumberVariant>(&height->second))
		{
			this->setHeight(item->value());
		}
	}

	auto width = propDict.find(FLV_AMF_PROP_WIDTH);
	if (width != propDict.end())
	{
		if (const auto* item = boost::get<AMF::NumberVariant>(&width->second))
		{
			this->setWidth(item->value());
		}
	}

	auto videoDataRate = propDict.find(FLV_AMF_PROP_VIDEODATARATE);
	if (videoDataRate != propDict.end())
	{
		if (const auto* item = boost::get<AMF::NumberVariant>(&videoDataRate->second))
		{
			this->setVideoDataRate(item->value());
		}
	}

	auto frameRate = propDict.find(FLV_AMF_PROP_FRAMERATE);
	if (frameRate != propDict.end())
	{
		if (const auto* item = boost::get<AMF::NumberVariant>(&frameRate->second))
		{
			this->setFrameRate(item->value());
		}
	}

	auto videoCodecId = propDict.find(FLV_AMF_PROP_VIDEOCODECID);
	if (videoCodecId != propDict.end())
	{
		if (const auto* item = boost::get<AMF::NumberVariant>(&videoCodecId->second))
		{
			this->setVideoCodecId(item->value());
		}
	}

	auto audioDataRate = propDict.find(FLV_AMF_PROP_AUDIODATARATE);
	if (audioDataRate != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&audioDataRate->second))
		{
			this->setAudioDataRate(item->value());
		}
	}

	auto audioSampleRate = propDict.find(FLV_AMF_PROP_AUDIOSAMPLERATE);
	if (audioSampleRate != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&audioSampleRate->second))
		{
			this->setAudioSampleRate(item->value());
		}
	}

	auto audioSampleSize = propDict.find(FLV_AMF_PROP_AUDIOSAMPLESIZE);
	if (audioSampleSize != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&audioSampleSize->second))
		{
			this->setAudioSampleSize(item->value());
		}
	}

	auto stereo = propDict.find(FLV_AMF_PROP_STEREO);
	if (stereo != propDict.end())
	{
		if (const auto *item = boost::get<AMF::BooleanVariant>(&stereo->second))
		{
			this->setStereo(item->value());
		}
	}

	auto audioCodecId = propDict.find(FLV_AMF_PROP_AUDIOCODECID);
	if (audioCodecId != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&audioCodecId->second))
		{
			this->setAudioCodecId(item->value());
		}
	}

	auto encoder = propDict.find(FLV_AMF_PROP_ENCODER);
	if (encoder != propDict.end())
	{
		if (const auto *item = boost::get<AMF::StringVariant>(&encoder->second))
		{
			this->setEncoder(item->value());
		}
	}

	auto fileSize = propDict.find(FLV_AMF_PROP_FILESIZE);
	if (fileSize != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&fileSize->second))
		{
			this->setFileSize(item->value());
		}
	}

	auto avcLevel = propDict.find(FLV_AMF_PROP_AVCLEVEL);
	if (avcLevel != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&avcLevel->second))
		{
			this->setAvcLevel(item->value());
		}
	}

	auto avcProfile = propDict.find(FLV_AMF_PROP_AVCPROFILE);
	if (avcProfile != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&avcProfile->second))
		{
			this->setAvcProfile(item->value());
		}
	}
	
	auto audioChannels = propDict.find(FLV_AMF_PROP_AUDIOCHANNELS);
	if (audioChannels != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&audioChannels->second))
		{
			this->setAudioChannels(item->value());
		}
	}
	
	auto videoFrameRate = propDict.find(FLV_AMF_PROP_VIDEOFRAMERATE);
	if (videoFrameRate != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&videoFrameRate->second))
		{
			this->setVideoFrameRate(item->value());
		}
	}

	auto timestamp = propDict.find(FLV_AMF_PROP_TIMESTAMP);
	if (timestamp != propDict.end())
	{
		if (const auto *item = boost::get<AMF::NumberVariant>(&timestamp->second))
		{
			this->setTimestamp(item->value());
		}
	}
}

// TODO(diegostamigni): split into different methods
void FLVMediaAMFTag::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	FLVMediaTag::deserialize(begin, end);
	if (payload_.size() <= 0) return;

	deserialize();
}

const v8& FLVMediaAMFTag::serialize()
{
	payload_.clear();

	const auto serializer = AMF::v0::Serializer{};

	auto props = AMF::TDictSubset{};
	props[FLV_AMF_PROP_STEREO] = AMF::BooleanVariant(stereo());

	auto tag = serializer.serializeString(FLV_AMF_ONMETADATA);
	std::move(tag.begin(), tag.end(), std::back_inserter(payload_));

	if (duration() > 0)
	{
		props[FLV_AMF_PROP_DURATION] = AMF::NumberVariant(duration());
	}

	if (width() > 0)
	{
		props[FLV_AMF_PROP_WIDTH] = AMF::NumberVariant(width());
	}

	if (height() > 0)
	{
		props[FLV_AMF_PROP_HEIGHT] = AMF::NumberVariant(height());
	}

	if (videoDataRate() > 0)
	{
		props[FLV_AMF_PROP_VIDEODATARATE] = AMF::NumberVariant(videoDataRate());
	}

	if (frameRate() > 0)
	{
		props[FLV_AMF_PROP_FRAMERATE] = AMF::NumberVariant(frameRate());
	}

	if (audioDataRate() > 0)
	{
		props[FLV_AMF_PROP_AUDIODATARATE] = AMF::NumberVariant(audioDataRate());
	}

	if (audioSampleSize() > 0)
	{
		props[FLV_AMF_PROP_AUDIOSAMPLESIZE] = AMF::NumberVariant(audioSampleSize());
	}

	if (audioSampleRate() > 0)
	{
		props[FLV_AMF_PROP_AUDIOSAMPLERATE] = AMF::NumberVariant(audioSampleRate());
	}

	if (!encoder().empty())
	{
		props[FLV_AMF_PROP_ENCODER] = AMF::StringVariant(encoder());
	}

	if (fileSize() > 0)
	{
		props[FLV_AMF_PROP_FILESIZE] = AMF::NumberVariant(fileSize());
	}

	if (videoCodecId() > 0)
	{
		props[FLV_AMF_PROP_VIDEOCODECID] = AMF::NumberVariant(videoCodecId());
	}

	if (!videoCodecIdStr().empty())
	{
		props[FLV_AMF_PROP_VIDEOCODECID] = AMF::StringVariant(videoCodecIdStr());
	}

	if (!audioCodecIdStr().empty())
	{
		props[FLV_AMF_PROP_AUDIOCODECID] = AMF::StringVariant(audioCodecIdStr());
	}

	if (audioChannels() > 0)
	{
		props[FLV_AMF_PROP_AUDIOCHANNELS] = AMF::NumberVariant(audioChannels());
	}

	if (videoFrameRate() > 0)
	{
		props[FLV_AMF_PROP_VIDEOFRAMERATE] = AMF::NumberVariant(videoFrameRate());
	}

	if (timestamp() > 0)
	{
		props[FLV_AMF_PROP_TIMESTAMP] = AMF::NumberVariant(timestamp());
	}
	
	if (avcLevel() > 0 && (!videoCodecIdStr().empty() && videoCodecIdStr() == "avc1"))
	{
		props[FLV_AMF_PROP_AVCLEVEL] = AMF::NumberVariant(avcLevel());
	}
	
	if (avcProfle() > 0 && (!videoCodecIdStr().empty() && videoCodecIdStr() == "avc1"))
	{
		props[FLV_AMF_PROP_AVCPROFILE] = AMF::NumberVariant(avcProfle());
	}

	auto serialized = serializer.serializeObject(props);
	std::move(serialized.begin(), serialized.end(), std::back_inserter(payload_));

	return payload_;
}
