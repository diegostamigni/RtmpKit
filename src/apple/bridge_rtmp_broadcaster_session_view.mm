#import "../../include/apple/KBRtmpBroadcasterSessionView.h"

#import "../../private/apple/KBAVAudioSessionManager.h"
#import "../../private/apple/KBVideoCaptureSessionView.h"
#import "../../private/apple/KBAudioCaptureSession.h"
#import "../../private/apple/KBUtility.h"

#include "../../private/flv/flv_amf_mediatag.h"
#include "../../private/rtmp/session/rtmp_broadcaster_session.h"
#include "../../private/queue.h"
#include "../../private/logger.h"

using RtmpSample = std::tuple<RtmpKit::FLVVideoFrameType, RtmpKit::u32, RtmpKit::v8, bool, RtmpKit::RtmpMessageType>;
struct RtmpBroadcasterDelegateCollector;

@interface KBRtmpBroadcasterSessionView() <
	KBVideoCompressorSessionDelegate,
	KBAudioCaptureSessionDelegate,
	KBAVAudioSessionManagerDelegate,
	KBReconnectableSession
>
{
	BOOL shouldSendAudioParameters;
}

@property (nonatomic, strong) KBAudioCaptureSession *captureAudioSession;
@property (nonatomic, strong) KBVideoCaptureSessionView *captureVideoSessionView;

@property (nonatomic, assign, getter=isSessionReady) BOOL sessionReady;

@property (nonatomic, assign) std::shared_ptr<RtmpBroadcasterDelegateCollector> rtmpSessionDelegate;
@property (nonatomic, assign) std::shared_ptr<RtmpKit::RtmpBroadcasterSession> rtmpSession;
@property (nonatomic, assign) std::shared_ptr<RtmpKit::Queue<RtmpSample>> queue;

@property (nonatomic, assign) int32_t prevVideoTimestamp;
@property (nonatomic, assign) int32_t prevAudioTimestamp;

@property (nonatomic, assign) double calculatedBitRate;
@property (nonatomic, assign) bool shouldReconnect;
@property (nonatomic, assign) bool reconnecting;
@property (nonatomic, assign, getter=hasStopBeenRequested) bool stopRequested;

@property (nonatomic, strong) NSString *host;
@property (nonatomic, strong) NSString *port;
@property (nonatomic, strong) NSString *appName;
@property (nonatomic, strong) NSString *streamURL;

@end

// PIMPL
struct RtmpBroadcasterDelegateCollector : RtmpKit::RtmpBroadcasterSessionDelegate
{
	explicit RtmpBroadcasterDelegateCollector(KBRtmpBroadcasterSessionView *objc)
		: objc_(objc)
	{
	}
	
	void socketWillConnect() override
	{
	}
	
	void socketDidConnect() override
	{
		objc_.shouldReconnect = false;
	}
	
	void socketWillClose() override
	{
	}
	
	void socketDidClose() override
	{
		if (objc_.shouldReconnect)
		{
			[objc_ reconnectAfter:3];
		}
		else
		{
			[objc_ end];
		}
	}
	
	void socketFailedToConnect(const std::string& errMessage) override
	{
	}
	
	void reconnectionRequired() override
	{
		if (!objc_.reconnecting)
		{
			LOG_INFO("reconnectionRequired()");
			objc_.shouldReconnect = true;
		}
	}
	
	void handshakeDone() override
	{
	}
	
	void sessionStarted() override
	{
		LOG_INFO("sessionStarted()");
		
		auto tag = RtmpKit::FLVMediaAMFTag{};
		tag.setAudioChannels([objc_.rtmpSettings audioChannels]);
		tag.setFrameRate([objc_.rtmpSettings frameRate]);
		tag.setVideoFrameRate([objc_.rtmpSettings keyFrameInterval]);
		tag.setVideoDataRate([objc_.rtmpSettings videoDataRate]);
		tag.setWidth([objc_.rtmpSettings width]);
		tag.setHeight([objc_.rtmpSettings height]);
		tag.setAudioSampleRate(kSampleRate);
		objc_.rtmpSession->sendOnMetaDataMessage(std::move(tag));
		
		[objc_.captureVideoSessionView reset];
        [objc_.captureAudioSession reset];
        objc_.sessionReady = true;
	}
	
	void networkPerformanceMetricsChanged(RtmpKit::NetworkType type, std::size_t preferredByterate) override
	{
		if (![objc_.rtmpSettings isDynamicVideoBitRateEnabled] || preferredByterate == 0) return;
		
		auto shouldReset = false;
		const double defaultBitrate = objc_.rtmpSettings.videoDataRate * 1024., newBitrate = preferredByterate * 8;
		
		switch (type)
		{
			case RtmpKit::NetworkType::Highest:
			{
				shouldReset = true;
				break;
			}
				
			default:
				break;
		}
		
		if (shouldReset && objc_.calculatedBitRate != defaultBitrate)
		{
			objc_.calculatedBitRate = [[objc_.captureVideoSessionView videoCompressorSession] resetCompressorBitRate];
			objc_.rtmpSession->updateCurrentMetrics(std::ceil(static_cast<std::size_t>(objc_.calculatedBitRate) / 8));

			if ([objc_.delegate respondsToSelector:@selector(dynamicBitRateDidChange:)]) {
				dispatch_async(dispatch_get_main_queue(), ^{
					[objc_.delegate dynamicBitRateDidChange:objc_.calculatedBitRate];
				});
			}
			
			LOG_INFO(boost::format("Video bitrate reset to %1%, network type: %2%")
				% static_cast<std::size_t>(objc_.calculatedBitRate)
				% type);
		}
		else
		{
			[objc_.captureVideoSessionView.videoCompressorSession updateCompressorBitRate: newBitrate];
			objc_.rtmpSession->updateCurrentMetrics(static_cast<std::size_t>(preferredByterate));
			objc_.calculatedBitRate = newBitrate;
			
			if ([objc_.delegate respondsToSelector:@selector(dynamicBitRateDidChange:)]) {
				dispatch_async(dispatch_get_main_queue(), ^{
					[objc_.delegate dynamicBitRateDidChange:objc_.calculatedBitRate];
				});
			}
			
			LOG_INFO(boost::format("Video bitrate changed to %1%, network type: %2%")
				% static_cast<std::size_t>(objc_.calculatedBitRate)
				% type);
		}
	}
	
private:
	KBRtmpBroadcasterSessionView __weak * objc_;
};

@implementation KBRtmpBroadcasterSessionView

#if defined TARGET_IOS
- (instancetype)initWithCoder: (NSCoder *)aDecoder
{
	if (self = [super initWithCoder:aDecoder])
	{
		self.captureVideoSessionView = [[KBVideoCaptureSessionView alloc] initWithCoder:aDecoder];
		[self setup];
	}
	
	return self;
}
#endif

- (instancetype)initWithFrame:(CGRect)frame
{
	if (self = [super initWithFrame: frame])
	{
		self.captureVideoSessionView = [[KBVideoCaptureSessionView alloc] initWithFrame:frame];
		[self setup];
	}
	
	return self;
}

- (instancetype)init
{
	if (self = [super init])
	{
		self.captureVideoSessionView = [[KBVideoCaptureSessionView alloc] init];
		[self setup];
	}
	
	return self;
}

- (void)setSessionReady:(BOOL)sessionReady
{
    if (self.delegate)
    {
        if (!sessionReady && [self.delegate respondsToSelector:@selector(sessionEnded)])
        {
            dispatch_async(dispatch_get_main_queue(), ^{ [self.delegate sessionEnded]; });
        }
        else if(sessionReady && [self.delegate respondsToSelector:@selector(sessionStarted)])
        {
            dispatch_async(dispatch_get_main_queue(), ^{ [self.delegate sessionStarted]; });
        }
    }
    
    _sessionReady = sessionReady;
	_reconnecting = !sessionReady;
}

- (void) realign
{
}

- (void)setAudioOnly:(BOOL)audioOnly
{
	_audioOnly = audioOnly;
	self.captureVideoSessionView.hidden = _audioOnly;
	
	if ([self isRunning])
	{
		if (_audioOnly)
		{
			[self.captureVideoSessionView end];
		}
		else
		{
			[self.captureVideoSessionView startIfNeeded];
		}
	}
}

- (void) setup
{
	self.captureAudioSession = [KBAudioCaptureSession new];
	[self setupNative];
	
	self.prevVideoTimestamp = 0;
	self.prevAudioTimestamp = 0;
	
	// TODO(diegostamigni): we should have a multicast delegate here..
	[[KBAVAudioSessionManager sharedInstance] setDelegate:self];
	
	self.backgroundColor = [UIColor clearColor];
	
	[self addSubview:self.captureVideoSessionView];
	[self.captureVideoSessionView setTranslatesAutoresizingMaskIntoConstraints: false];
	
	id target = @{@"captureVideoSessionView" : self.captureVideoSessionView};
	
	[self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat: @"|[captureVideoSessionView]|"
																 options: kNilOptions
																 metrics: nil
																   views: target]];
	
	[self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat: @"V:|[captureVideoSessionView]|"
																 options: kNilOptions
																 metrics: nil
																   views: target]];
}

- (void) setupNative
{
	self.rtmpSession = std::make_shared<RtmpKit::RtmpBroadcasterSession>();
	
	self.rtmpSessionDelegate = std::make_shared<RtmpBroadcasterDelegateCollector>(self);
	self.rtmpSession->addBroadcasterDelegate(self.rtmpSessionDelegate);
	
	[self setupQueue];
}

- (void) setupQueue
{
	const auto handler = [
		rs = self.rtmpSession,
		ac = self.captureAudioSession,
		vc = self.captureVideoSessionView,
		prevVideoTs = &_prevVideoTimestamp,
		prevAudioTs = &_prevAudioTimestamp,
		sr = &_sessionReady] (auto&& item)
	{
		if (rs.get() == nullptr || *sr == false) return;
		
		const auto timestamp = std::get<RtmpKit::u32>(item);
		const auto isParam = std::get<bool>(item);
		
		auto data = std::move(std::get<RtmpKit::v8>(item));
		if (data.empty()) return;
		
		switch (std::get<RtmpKit::RtmpMessageType>(item))
		{
			case RtmpKit::RtmpMessageType::VideoMessage:
			{
				rs->sendVideoMessage(std::get<RtmpKit::FLVVideoFrameType>(item), timestamp, std::move(data), isParam);
				*prevVideoTs = timestamp;
				break;
			}
				
			case RtmpKit::RtmpMessageType::AudioMessage:
			{
				rs->sendAudioMessage(timestamp, std::move(data), isParam);
				*prevAudioTs = timestamp;
				break;
			}
				
			default:
				break;
		}
		
		const auto audioVideoDistance = (*prevAudioTs - *prevVideoTs);
		const auto audioVideoOutOfSync = std::abs(audioVideoDistance) >= kAudioVideoAllowedDelay;
		
		if (audioVideoOutOfSync)
		{
			LOG_DEBUG(boost::format("Audio/Video is out-of-sync: %1%") % audioVideoDistance);
			
			[ac reset];
			[vc reset];
		}
	};
	
	self.queue = std::make_shared<RtmpKit::Queue<RtmpSample>>(handler);
}

- (void)setLayoutForOnStage:(BOOL)layoutForOnStage
{
	_layoutForOnStage = layoutForOnStage;
	self.captureVideoSessionView.layoutForOnStage = layoutForOnStage;
}

- (void)setOnStageSide:(KBVideoCaptureSessionViewOnStageSide)onStageSide
{
	_onStageSide = onStageSide;
	self.captureVideoSessionView.onStageSide = onStageSide;
}

- (void)startWithHost: (NSString *) host
			  andPort: (NSString *) port
		   andAppName: (NSString *) appName
		 andStreamURL: (NSString *) streamURL
	 layoutForOnStage: (BOOL) onStage
{
	self.layoutForOnStage = onStage;
	
	dispatch_async(dispatch_get_main_queue(),
	^{
		if (![self isRunning])
		{
			self.host = host;
			self.port = port;
			self.appName = appName;
			self.streamURL = streamURL;
			
			[self markAsRunning];
			self.queue->start();
			
			self.rtmpSession->start(std::string{[host cStringUsingEncoding:NSUTF8StringEncoding]},
									std::string{[port cStringUsingEncoding:NSUTF8StringEncoding]},
									std::string{[appName cStringUsingEncoding:NSUTF8StringEncoding]},
									std::string{[streamURL cStringUsingEncoding:NSUTF8StringEncoding]});
			
			self.captureVideoSessionView.hidden = [self isAudioOnly];
			
			if (![self isAudioOnly])
			{
				[self.captureVideoSessionView startIfNeeded];
				[self.captureVideoSessionView setDelegate:self];
			}
			
			[self.captureAudioSession startIfNeeded];
			[self.captureAudioSession setDelegate:self];
		}
	});
}

- (void)end
{
	dispatch_async(dispatch_get_main_queue(),
	^{
		_stopRequested = true;
		_shouldReconnect = false;
					   
		[self markAsStopped];
		self.rtmpSession->end();
			
		self.queue->stop();
		self.queue->clear();
		
		[self.captureVideoSessionView end];
		[self.captureAudioSession end];
	});
}

- (void) reconnectAfter:(NSInteger) waitingTimeInSeconds
{
	auto handler = [waitingTimeInSeconds, self]()
	{
		if (!self.shouldReconnect || self.reconnecting) return;
		self.reconnecting = true;
		
		if (self.host && self.port && self.appName && self.streamURL && ![self hasStopBeenRequested])
		{
			[self.delegate sessionIsReconnecting];
			[self clear];
			
			_sessionReady = false;
			self.rtmpSession->end();
			self.queue->stop();
			self.queue->clear();
			
			[self.captureVideoSessionView end];
			[self.captureAudioSession end];
			
			dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
				(int64_t)(waitingTimeInSeconds * NSEC_PER_SEC)), dispatch_get_main_queue(),
			^{
				[self startWithHost:self.host
							andPort:self.port
						 andAppName:self.appName
					   andStreamURL:self.streamURL
				   layoutForOnStage:self.layoutForOnStage];
			});
		}
	};
	
	if (waitingTimeInSeconds <= 0)
	{
		dispatch_async(dispatch_get_main_queue(),
		^{
			handler();
		});
	}
	else
	{
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW,
			(int64_t)(waitingTimeInSeconds * NSEC_PER_SEC)), dispatch_get_main_queue(),
		^{
			handler();
		});
	}
}

- (void) markAsRunning
{
	_running = true;
	_stopRequested = false;
	shouldSendAudioParameters = true;
}

- (void) markAsStopped
{
	self.sessionReady = false;
	[self clear];
}

- (void) clear
{
	_running = false;
	shouldSendAudioParameters = false;
	
	self.prevAudioTimestamp = 0;
	self.prevVideoTimestamp = 0;
	self.shouldReconnect = false;
	
	// After a reconnection, do we want to clear the queued media items ?
	// If so, we we'll loose data while the BK was reconnecting
	self.queue->clear();
}

- (void)switchCamera
{
	[self.captureVideoSessionView switchCamera];
}

- (void)setRtmpSettings:(KBRtmpSessionSettings *)rtmpSettings
{
    _rtmpSettings = rtmpSettings;
	_calculatedBitRate = [self.rtmpSettings videoDataRate] * 1024.;
    [self.captureAudioSession setRtmpSettings: rtmpSettings];
	[self.captureVideoSessionView setRtmpSettings: rtmpSettings];
}

- (void)setMirrorHorizontally:(BOOL)mirrorHorizontally
{
	[self.captureVideoSessionView setMirrorHorizontally:mirrorHorizontally];
	_mirrorHorizontally = [self.captureVideoSessionView isMirroredHorizontally];
}

#pragma mark - KBVideoCompressorSessionDelegate

- (void)videoSampleEncoded:(CMSampleBufferRef)compressedSample
	  withPresentationTime:(CMTime)presentationTime
			   andDuration:(CMTime)duration
{
	if (![self isSessionReady]) return;
	
	if (auto raw = [KBUtility bufferToRaw: compressedSample])
	{
		const auto timestamp = static_cast<RtmpKit::u24>(CMTimeGetSeconds(presentationTime) * 1000);
		const auto frameType = [KBUtility isKeyFrame: compressedSample]
			? RtmpKit::FLVVideoFrameType::KeyFrame
			: RtmpKit::FLVVideoFrameType::InterFrame;
		
		if (frameType == RtmpKit::FLVVideoFrameType::KeyFrame)
		{
			auto data = [KBUtility buildParametersVideoPacketCompressedSample:compressedSample];
			self.queue->push({frameType, timestamp, data, true, RtmpKit::RtmpMessageType::VideoMessage});
		}
		
		const auto lenght = [KBUtility bufferSize: compressedSample];
		self.queue->push({frameType, timestamp, RtmpKit::v8(raw, raw + lenght), false, RtmpKit::RtmpMessageType::VideoMessage});
	}
}

#pragma mark - KBAudioCaptureSessionDelegate

- (void)audioSampleReceived:(const uint8_t * _Nullable)raw
				 withLenght:(size_t)lenght
	   withPresentationTime:(uint32_t)timestamp
				andDuration:(CMTime)duration
{
	if (![self isSessionReady]) return;
	
	self.queue->push({RtmpKit::FLVVideoFrameType::Unknown, timestamp,
		shouldSendAudioParameters ? [KBUtility buildParametersAudioPacket] : RtmpKit::v8(raw, raw + lenght),
			shouldSendAudioParameters, RtmpKit::RtmpMessageType::AudioMessage});
	
	if (shouldSendAudioParameters) shouldSendAudioParameters = false;
}

#pragma mark: - KBAVAudioSessionManagerDelegate

- (void)audioSessionManager:(KBAVAudioSessionManager *)audioManager
	 audioInterruptionBegan:(AVAudioSessionInterruptionType)type
{
	[self.captureVideoSessionView end];
}

- (void)audioSessionManager:(KBAVAudioSessionManager *)audioManager
	 audioInterruptionEnded:(AVAudioSessionInterruptionType)type
{
	[self.captureAudioSession end];
	[self setShouldReconnect:true];
}

- (void)audioSessionManager:(KBAVAudioSessionManager *)audioManager
audioInterruptionShouldResume:(AVAudioSessionInterruptionOptions)type
{
	[self reconnectAfter: 0];
}

@end
