#import "../../include/apple/KBRtmpWatcherSessionView.h"

#import "../../private/apple/KBOpenGLBasedPreviewView.h"
#import "../../private/apple/KBVideoDecompressorSession.h"
#import "../../private/apple/KBAudioRendererSession.h"
#import "../../private/apple/KBUtility.h"

#include "../../private/flv/flv_amf_mediatag.h"
#include "../../private/rtmp/session/rtmp_watcher_session.h"
#include "../../private/logger.h"

#import <AudioToolbox/AudioToolbox.h>

#define FRAME_START_PREFIX_SIZE 4
#define NALU_START_PREFIX_SIZE 3
#define NAL_LENGTH_PREFIX_SIZE 4
#define NAL_TYPE_SPS 0x7
#define NAL_TYPE_PPS 0x8

struct RtmpWatcherDelegateCollector;

@interface KBRtmpWatcherSessionView() <KBVideoDecompressorSessionDelegate, KBReconnectableSession>

@property (nonatomic, assign, getter=isSessionReady) BOOL sessionReady;
@property (nonatomic, strong) KBOpenGLBasedPreviewView *openGlView;
@property (nonatomic, strong) KBVideoDecompressorSession *videoDecompressor;
@property (nonatomic, strong) KBAudioRendererSession *audioPlayback;

@property (nonatomic, assign) std::shared_ptr<RtmpWatcherDelegateCollector> rtmpSessionDelegate;
@property (nonatomic, assign) std::shared_ptr<RtmpKit::RtmpWatcherSession> rtmpSession;

@property (nonatomic, assign) bool shouldReconnect;
@property (nonatomic, assign) bool reconnecting;
@property (nonatomic, assign, getter=hasStopBeenRequested) bool stopRequested;

@property (nonatomic, strong) NSString *host;
@property (nonatomic, strong) NSString *port;
@property (nonatomic, strong) NSString *appName;
@property (nonatomic, strong) NSString *streamURL;

- (void) updateRtmpSettings;

@end

// PIMPL
struct RtmpWatcherDelegateCollector : RtmpKit::RtmpWatcherSessionDelegate
{
	explicit RtmpWatcherDelegateCollector(KBRtmpWatcherSessionView *objc)
		: objc_(objc)
		, videoTimebase_(0)
		, audioTimebase_(0)
		, formatDescription_(nullptr)
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
		objc_.sessionReady = true;
	}
	
	void metadataPacketReceived(RtmpKit::RtmpMetadataMessage&& packet) override
	{
		if (!objc_.rtmpSettings)
		{
			objc_.rtmpSettings = [KBRtmpSessionSettings new];
		}
		
		[objc_.rtmpSettings setWidth:packet.height()];
		[objc_.rtmpSettings setHeight:packet.width()];
		[objc_.rtmpSettings setFrameRate:packet.frameRate()];
		[objc_.rtmpSettings setAudioChannels:packet.audioChannels()];
		
		[objc_ updateRtmpSettings];
	}
	
	void videoPacketReceived(RtmpKit::RtmpVideoMessage&& packet) override
	{
		[KBUtility unpackVideoPacket:std::forward<RtmpKit::RtmpVideoMessage>(packet)
						withTimebase:std::ref(videoTimebase_)
				andFormatDescription:std::ref(formatDescription_)
						  completion:^(CMSampleBufferRef sampleBuffer)
		 {
			 [objc_.videoDecompressor decodeFrameWithSampleBuffer:sampleBuffer];
		 }];
	}
	
	void audioPacketReceived(RtmpKit::RtmpAudioMessage&& packet) override
	{
		[KBUtility unpackAudioPacket:std::forward<RtmpKit::RtmpAudioMessage>(packet)
						withTimebase:std::ref(audioTimebase_)
					  newAudioFormat:^(AudioStreamBasicDescription audioFormat)
		 {
			 [objc_.audioPlayback setAudioFormat:audioFormat];
			 [objc_.audioPlayback startIfNeeded];
		 }
		 completion:^(RtmpKit::v8&& data, CMTime timestamp)
		 {
			 [objc_.audioPlayback appendAudioData:std::move(data) withPresentationTime:timestamp];
		 }];
	}
	
	void networkPerformanceMetricsChanged(RtmpKit::NetworkType type, std::size_t preferredByterate) override
	{
		[objc_.audioPlayback networkTypeChanged:static_cast<KBNetworkType>(type)];
	}
	
private:
	KBRtmpWatcherSessionView __weak * objc_;
	RtmpKit::u32 videoTimebase_;
	RtmpKit::u32 audioTimebase_;
	CMFormatDescriptionRef formatDescription_;
};

@implementation KBRtmpWatcherSessionView

#if defined TARGET_IOS
- (instancetype)initWithCoder: (NSCoder *)aDecoder
{
	self = [super initWithCoder:aDecoder];
	
	if (self)
	{
		[self setup];
	}
	
	return self;
}
#endif

- (instancetype)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame: frame];
	
	if (self)
	{
		[self setup];
	}
	
	return self;
}

- (instancetype)init
{
	self = [super init];
	
	if (self)
	{
		[self setup];
	}
	
	return self;
}

- (void) setup
{
	[self setupNative];
	self.backgroundColor = [UIColor clearColor];
	
	self.videoDecompressor = [KBVideoDecompressorSession new];
	self.videoDecompressor.delegate = self;
	
	self.openGlView = [[KBOpenGLBasedPreviewView alloc] init];
	[self addSubview:self.openGlView];
	
    self.audioPlayback = [KBAudioRendererSession new];
	self.audioPlayback.delegate = self.openGlView;
	
	id target = @{@"openGlView" : self.openGlView};
	[self.openGlView setTranslatesAutoresizingMaskIntoConstraints: false];
	
	[self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat: @"|[openGlView]|"
																 options: kNilOptions
																 metrics: nil
																   views: target]];
	
	[self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat: @"V:|[openGlView]|"
																 options: kNilOptions
																 metrics: nil
																   views: target]];
}

- (void)willMoveToWindow:(UIWindow *)newWindow
{
	if (newWindow)
	{
#if defined TARGET_IOS
		[[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationDidReceiveMemoryWarningNotification
														  object:[UIApplication sharedApplication]
														   queue:nil
													  usingBlock:^(NSNotification *notif)
		 {
			 [self setStopRequested:false];
			 [self setShouldReconnect:true];
			 [self reconnectAfter:0];
		 }];
#endif
	}
}

- (void)didMoveToWindow
{
	if (self.window == nil)
	{
		[[NSNotificationCenter defaultCenter] removeObserver:self];
	}
}

- (void) setupNative
{
	self.rtmpSession = std::make_shared<RtmpKit::RtmpWatcherSession>();
	self.rtmpSessionDelegate = std::make_shared<RtmpWatcherDelegateCollector>(self);
	self.rtmpSession->addWatcherDelegate(self.rtmpSessionDelegate);
}

- (void)setLayoutForOnStage:(BOOL)layoutForOnStage
{
	_layoutForOnStage = layoutForOnStage;
	self.openGlView.layoutForOnStage = layoutForOnStage;
}

- (void) realign
{
	LOG_INFO("Audio/Video cached buffer is realigning..");
	
	[self.audioPlayback reset];
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
	
	_reconnecting = !sessionReady;
	_sessionReady = sessionReady;
}

- (void) startWithHost:(NSString *)host
			   andPort:(NSString *)port
			andAppName:(NSString *)appName
		  andStreamURL:(NSString *)streamURL
	  layoutForOnStage:(BOOL)onStage
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
			[self.openGlView start];
			
			self.rtmpSession->start(std::string{[host cStringUsingEncoding:NSUTF8StringEncoding]},
									std::string{[port cStringUsingEncoding:NSUTF8StringEncoding]},
									std::string{[appName cStringUsingEncoding:NSUTF8StringEncoding]},
									std::string{[streamURL cStringUsingEncoding:NSUTF8StringEncoding]});
        }
	});
}

- (void) end
{
	dispatch_async(dispatch_get_main_queue(),
	^{
		if ([self isRunning])
		{
			_stopRequested = true;
			_shouldReconnect = false;
			
			[self.openGlView end];
			[self.audioPlayback end];
			[self.videoDecompressor end];
			[self markAsStopped];
			
			self.rtmpSession->end();
		}
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
			
			_running = false;
			_sessionReady = false;
			_shouldReconnect = false;
			
			self.rtmpSession->end();
			
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
}

- (void) markAsStopped
{
	_running = false;
	
	self.shouldReconnect = false;
	self.sessionReady = false;
}

- (void)setRtmpSettings:(KBRtmpSessionSettings *)rtmpSettings
{
	_rtmpSettings = rtmpSettings;
	[self updateRtmpSettings];
}

- (void) updateRtmpSettings
{
	self.audioPlayback.rtmpSettings = self.rtmpSettings;
	self.openGlView.rtmpSettings = self.rtmpSettings;
}

- (void)setMirrorHorizontally:(BOOL)mirrorHorizontally
{
	_mirrorHorizontally = mirrorHorizontally;
	self.openGlView.mirrorHorizontally = mirrorHorizontally;
}

#pragma mark - KBVideoDecompressorSessionDelegate

- (void)videoSampleDecoded:(CIImage *)image
	  withPresentationTime:(CMTime)presentationTime
			   andDuration:(CMTime)duration
			  withTimebase:(CMTimebaseRef)timeBase
{
	[self.openGlView appendImage:image withPresentationTime:presentationTime andTimebase: timeBase];
}

@end
