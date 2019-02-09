#import "../../private/apple/KBOpenGLBasedPreviewView.h"
#import "../../private/apple/KBUtility.h"

#include "../../private/queue.h"
#include "../../private/logger.h"

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
#endif

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <CoreMedia/CoreMedia.h>

using QueueItem = std::tuple<CMTime, CIImage*>;

@interface KBOpenGLBasedPreviewView()
{
	CIImage *lastImage;
	std::mutex mutex;
}

@property (nonatomic, strong) CIContext *ciContext;
@property (nonatomic, strong) NSThread *rendererThread;
@property (nonatomic, strong) CADisplayLink *displayLink;
@property (nonatomic, assign) CMTimebaseRef timeBase;
@property (nonatomic, assign) std::shared_ptr<RtmpKit::Queue<QueueItem>> queue;

@property (nonatomic, assign) CMTime audioPresentationTime;
@property (nonatomic, assign) CMTime videoPresentationTime;

@end

@implementation KBOpenGLBasedPreviewView
@synthesize audioPresentationTime = _audioPresentationTime;
@synthesize videoPresentationTime = _videoPresentationTime;

static const std::vector<EAGLRenderingAPI> openGLVersions = {
	kEAGLRenderingAPIOpenGLES3,
	kEAGLRenderingAPIOpenGLES2,
	kEAGLRenderingAPIOpenGLES1
};

- (instancetype)init
{
	EAGLContext *ctx = nil;
	
	for (const auto &openGlVersion : openGLVersions)
	{
		EAGLContext *context = [[EAGLContext alloc] initWithAPI:openGlVersion];
		
		if (context)
		{
			ctx = context;
			break;
		}
	}
	
	self = [super initWithFrame:CGRectMake(0, 0, 1, 1) context:ctx];
	
	if (self)
	{
		[self setup];
	}
	
	return self;
}

- (CMTimebaseRef)timeBase
{
	if (!_timeBase)
	{
		CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, CMClockGetHostTimeClock(), &_timeBase);
		CMTimebaseSetTime(_timeBase, kCMTimeZero);
		CMTimebaseSetRate(_timeBase, 1);
	}
	
	return _timeBase;
}

- (void) setup
{
	self.delegate = self;
	self.renderingImages = true;
	self.queue = std::make_shared<RtmpKit::Queue<QueueItem>>();
	
	NSDictionary<NSString*, id> *options = @{
		kCIContextWorkingFormat : @(kCIFormatRGBA8),
		kCIContextUseSoftwareRenderer : @(false),
		kCIContextWorkingColorSpace : [NSNull new]
	};
	
	self.ciContext = [CIContext contextWithEAGLContext:self.context options:options];
}

- (void)setVideoPresentationTime:(CMTime)videoPresentationTime
{
	std::unique_lock<std::mutex> lk(mutex, std::defer_lock);
	if (lk.try_lock())
	{
		_videoPresentationTime = videoPresentationTime;
	}
}

- (void)setAudioPresentationTime:(CMTime)audioPresentationTime
{
	std::unique_lock<std::mutex> lk(mutex, std::defer_lock);
	if (lk.try_lock())
	{
		_audioPresentationTime = audioPresentationTime;
	}
}

- (CMTime)videoPresentationTime
{
	std::lock_guard<std::mutex> lk(mutex);
	return _videoPresentationTime;
}

- (CMTime)audioPresentationTime
{
	std::lock_guard<std::mutex> lk(mutex);
	return _audioPresentationTime;
}

- (void) invalidate
{
	if (self.displayLink != nil)
	{
		NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
		[runLoop cancelPerformSelectorsWithTarget:self];
		
		[self.displayLink removeFromRunLoop:runLoop forMode:NSDefaultRunLoopMode];
		[self.displayLink invalidate];
		
		self.displayLink = nil;
	}
	
	if (self.rendererThread != nil)
	{
		[self.rendererThread cancel];
		self.rendererThread = nil;
	}
}

- (void) render
{
	if (!self.displayLink)
	{
		NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
		
		self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(display)];
		[self.displayLink addToRunLoop:runLoop forMode:NSDefaultRunLoopMode];
		
		[self applyDisplayLinkFrameRate];
		
		[runLoop run];
	}
}

- (void) display
{
	if (self.superview != nil && self.displayLink != nil && self.rendererThread != nil)
	{
#if defined TARGET_IOS
		if ([UIApplication sharedApplication].applicationState == UIApplicationStateActive)
		{
			[super display];
		}
#endif
	}
	else
	{
		[self invalidate];
	}
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
	@autoreleasepool
	{
#if defined TARGET_IOS
		if ([UIApplication sharedApplication].applicationState != UIApplicationStateActive)
		{
			self.renderingImages = false;
			return;
		}
#endif
		
		// Obtaining the image to draw
		CIImage *image = [self nextVideoFrame];
		
		if (image != nil && self.mirrorHorizontally)
		{
			CGAffineTransform flipTransform = CGAffineTransformTranslate(CGAffineTransformMakeScale(-1, 1),
																		 image.extent.size.width,
																		 0);
			
			image = [image imageByApplyingTransform: flipTransform];
		}
		
		if (image != nil)
		{
			auto rect = [self drawingRectForImageExtent:image.extent];
			
			if ([self isLayoutForOnStage])
			{
				const auto offset = [self offsetForLayoutOnStage];
				rect = [KBUtility fixDrawingRectIfNeeded:rect inBounds:self.bounds withOffset:offset];
			}
			
			[self.ciContext drawImage:image inRect:rect fromRect:image.extent];
			
			if (!self.renderingImages)
			{
				self.renderingImages = true;
			}
		}
		else
		{
			if (self.renderingImages)
			{
				self.renderingImages = false;
			}
		}
		
		glFlush();
		[self.context presentRenderbuffer:GL_RENDERBUFFER];
	}
}

- (CGSize) offsetForLayoutOnStage
{
	const auto center = self.bounds.size.width / 2;
	auto offset = CGSizeMake(0, 0);
	
	if (self.mirrorHorizontally)
	{
		offset.width = (center / 2);
	}
	else
	{
		offset.width = (center / 2) * -1;
	}
	
	return offset;
}

- (CGRect) drawingRectForImageExtent: (CGRect) extent
{
	CGFloat scale = [[UIScreen mainScreen] scale];
	CGFloat width = self.bounds.size.width * scale;
	CGFloat height = self.bounds.size.height * scale;
	CGFloat fullWidth = height / self.layer.bounds.size.height * self.layer.bounds.size.width;
	CGFloat heightOffset = (self.layer.frame.size.height * scale) - height;
	CGFloat fullHeight = height;
	
	CGRect dst = CGRectMake(-(fullWidth - width) / 2, heightOffset, fullWidth, fullHeight);
	return dst;
	
//	CGRect rect = SAMRectForContentMode(dst, UIViewContentModeScaleAspectFill, extent);
//	return rect;
}

- (CIImage *) nextVideoFrame
{
	using namespace std::chrono_literals;
    CIImage *image = nil;
    
    while (!self.queue->empty())
	{
        const auto &peek = self.queue->front();
        const auto presentationTime = std::get<CMTime>(peek);
		const auto tuple = [self isVideoFrameTimestampAllowed:presentationTime];
		const auto isVideoFrameTimestampAllowed = std::get<0>(tuple);
		const auto shouldWait = std::get<1>(tuple);
		
        if (isVideoFrameTimestampAllowed)
        {
			auto items = self.queue->pop();
			image = std::move(std::get<CIImage*>(items));
			lastImage = nil;
			lastImage = image;
            break;
        }
		else if (!isVideoFrameTimestampAllowed && shouldWait)
		{
			break;
		}
        else if (!isVideoFrameTimestampAllowed && !shouldWait)
        {
			self.queue->removeWithPredicate([self](const auto& item)
			{
				const auto pTime = std::get<CMTime>(item);
				const auto result = [self isVideoFrameTimestampAllowed: pTime];
				const auto sw = std::get<1>(result);
				return !sw;
			});
        }
    }
	
	return image ? image : lastImage;
}

- (std::tuple<bool, bool>) // isVideoFrameAllowed, shouldWait
isVideoFrameTimestampAllowed: (CMTime) presentationTime
{
	self.videoPresentationTime = presentationTime;
	const auto timebase = self.audioPresentationTime;

	if (CMTIME_IS_VALID(timebase) && CMTIME_IS_VALID(presentationTime))
	{
		const auto videoTimestampMillis = CMTimeGetSeconds(presentationTime) * 1000;
		const auto timebaseMillis = CMTimeGetSeconds(timebase) * 1000;
		const auto delay = videoTimestampMillis - timebaseMillis;
		
		auto shouldWait = false;
		const auto absDelay = std::abs(delay);
		const auto isVideoFrameAllowed = absDelay <= kAudioVideoAllowedDelay;
		if (isVideoFrameAllowed) return std::make_tuple(isVideoFrameAllowed, shouldWait);
		
		if (delay > 0)
		{
			// this video frame is ahead to the current audio frame
			// which means that we should wait before rendering this sample (no pop)
			shouldWait = true;
		}
		else
		{
			// this video frame is behind to the current audio frame
			// which means that we should skip this sample (pop)
			shouldWait = false;
		}

		/*if (!isVideoFrameAllowed)
		{
			LOG_DEBUG(boost::format("%1% video frame: audio/video delay is %2%ms, max allowed is %3%ms")
				% (shouldWait ? "Waiting for presenting" : "Skipping")
				% (int32_t) delay
				% kAudioVideoAllowedDelay
			);
		}*/
		
		return std::make_tuple(isVideoFrameAllowed, shouldWait);
    }
	
    return {false, true};
}

- (void)setRtmpSettings:(KBRtmpSessionSettings *)rtmpSettings
{
	_rtmpSettings = rtmpSettings;
	
	[self applyDisplayLinkFrameRate];
}

- (void) applyDisplayLinkFrameRate
{
	const auto frameRate = self.rtmpSettings ? [self.rtmpSettings frameRate] : 30;
	const auto osv = [[NSProcessInfo processInfo] operatingSystemVersion];
	
	if (osv.majorVersion >= 10)
	{
		self.displayLink.preferredFramesPerSecond = frameRate;
	}
	else
	{
		self.displayLink.frameInterval = frameRate > 30 ? 1 : 2;
	}
}

- (void)setRenderingImages:(BOOL)renderingImages
{
	_renderingImages = renderingImages;
	self.hidden = !renderingImages;
	
	// TODO (diegostamigni): notify with a delegate for eventual spinner or something
}

- (void)end
{
	self.queue->clear();
	[self.rendererThread cancel];
	[self invalidate];
}

- (void)start
{
	self.queue->clear();
	
	self.rendererThread = [[NSThread alloc] initWithTarget:self selector:@selector(render) object:nil];
	[self.rendererThread start];
}

- (void) reset
{
    self.queue->clear();
}

- (void) appendImage: (CIImage*) image
withPresentationTime: (CMTime) presentationTime
		 andTimebase: (CMTimebaseRef)timeBase
{
	self.timeBase = timeBase;
	if (!image || !CMTIME_IS_VALID(presentationTime)) return;
	self.queue->push(std::make_tuple(presentationTime, image));
}

#pragma mark - KBAudioCaptureSessionDelegate

- (void)renderingAudioSampleAtPresentationTime:(CMTime)audioPresentationTime
{
	self.audioPresentationTime = audioPresentationTime;
}

- (BOOL)canRenderAudioSampleWithPresentationTime:(CMTime)audioPresentationTime
{
	self.audioPresentationTime = audioPresentationTime;
	return true;
}

@end
