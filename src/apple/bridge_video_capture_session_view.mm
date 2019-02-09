#import "../../private/apple/KBVideoCaptureSessionView.h"
#import "../../private/apple/KBVideoCaptureSession.h"
#import "../../private/apple/KBUtility.h"
#import "../../private/logger.h"

#import <QuartzCore/QuartzCore.h>

@interface KBVideoCaptureSessionView()
@property (nonatomic, strong) KBVideoCaptureSession *videoCaptureSession;
@end

@implementation KBVideoCaptureSessionView
@synthesize delegate = _delegate;
@synthesize mirrorHorizontally = _mirrorHorizontally;

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
	self.videoCaptureSession = [[KBVideoCaptureSession alloc] initUsingFrontCamera: true];
	self.videoCaptureSession.layerOutput.frame = [self bounds];
	[self.videoCaptureSession.layerOutput.connection setAutomaticallyAdjustsVideoMirroring:true];
	[self.layer addSublayer:self.videoCaptureSession.layerOutput];
}

- (void) fixCameraOrientation
{
	[self.videoCaptureSession fixCameraOrientation];
}

- (void)setDelegate:(id<KBVideoCompressorSessionDelegate>)delegate
{
	_delegate = delegate;
	self.videoCaptureSession.delegate = delegate;
}

- (id<KBVideoCompressorSessionDelegate>)delegate
{
	return _delegate;
}

- (KBVideoCompressorSession *)videoCompressorSession
{
	return [self.videoCaptureSession videoCompressorSession];
}

- (void)setMirrorHorizontally:(BOOL)mirrorHorizontally
{
	if ([self.videoCaptureSession.layerOutput.connection isVideoMirroringSupported])
	{
		_mirrorHorizontally = mirrorHorizontally;
		[self.videoCaptureSession.layerOutput.connection setAutomaticallyAdjustsVideoMirroring:false];
		[self.videoCaptureSession.layerOutput.connection setVideoMirrored:mirrorHorizontally];
	}
}

#if defined TARGET_IOS
- (void)layoutSubviews
{
	[super layoutSubviews];
#else
- (BOOL)needsLayout
{
	BOOL value = [super needsLayout];
	self.videoCaptureSession.layerOutput.frame = [self bounds];
#endif
#if defined TARGET_IOS
	const auto orientation = [[UIApplication sharedApplication] statusBarOrientation];
	
	if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
	{
		self.videoCaptureSession.layerOutput.videoGravity = AVLayerVideoGravityResizeAspectFill;
	}
	else
	{
		self.videoCaptureSession.layerOutput.videoGravity = AVLayerVideoGravityResizeAspect;
	}
	
	self.videoCaptureSession.layerOutput.frame = [self fixLayerForOnStageIfNeeded];
#endif
#if defined TARGET_IOS
}
#else
	return value;
}
#endif

- (void)setLayoutForOnStage:(BOOL)layoutForOnStage
{
	_layoutForOnStage = layoutForOnStage;
	
	self.videoCaptureSession.layerOutput.frame = [self fixLayerForOnStageIfNeeded];
}

- (void)setOnStageSide:(KBVideoCaptureSessionViewOnStageSide)onStageSide
{
	_onStageSide = onStageSide;
	
	self.videoCaptureSession.layerOutput.frame = [self fixLayerForOnStageIfNeeded];
}

- (CGSize) offsetForLayoutOnStage
{
	const auto center = self.bounds.size.width / 2;
	auto offset = CGSizeMake(0, 0);
	
	switch (self.onStageSide)
	{
		case KBVideoCaptureSessionViewOnStageSideLeft:
		{
			offset.width = center * -1;
			break;
		}
			
		case KBVideoCaptureSessionViewOnStageSideRight:
		{
			offset.width = (center / 4) * -1;
			break;
		}
			
		default:
			break;
	}
	
	return offset;
}

- (CGRect) fixLayerForOnStageIfNeeded
{
	const auto width = (self.bounds.size.height / 9 * 16);
	auto rect = CGRectMake(self.bounds.origin.x, self.bounds.origin.y, width, self.bounds.size.height);
	
	if ([self isLayoutForOnStage])
	{
		const auto offset = [self offsetForLayoutOnStage];
		rect = [KBUtility fixDrawingRectIfNeeded:rect inBounds:self.bounds withOffset:offset];
	}
	
	return rect;
}

- (void)switchCamera
{
	[self.videoCaptureSession switchCamera];
}

- (void)start
{
	[self.videoCaptureSession start];
}

- (void)startIfNeeded
{
	[self start];
}

- (void)reset
{
	[self.videoCaptureSession reset];
}

- (void)end
{
	[self.videoCaptureSession end];
}

- (void)setRtmpSettings:(KBRtmpSessionSettings *)rtmpSettings
{
	_rtmpSettings = rtmpSettings;
	[self.videoCaptureSession setRtmpSettings: rtmpSettings];
}

@end
