#import "../../private/apple/KBVideoCompressorSession.h"
#import "../../private/apple/KBUtility.h"
#include "../../private/logger.h"

#import <QuartzCore/QuartzCore.h>
#import <VideoToolbox/VideoToolbox.h>
#import <CoreImage/CoreImage.h>
#import <libkern/OSAtomic.h>

@interface KBVideoCompressorSession()
{
	CFTimeInterval lastTime;
	int32_t forceKeyFrameCounter;
}

@property (nonatomic, assign) VTCompressionSessionRef sessionRef;
@property (nonatomic, assign) CMVideoCodecType codecType;
@property (nonatomic, assign) NSInteger videoCapturedFrameIndex;
@property (nonatomic, assign) NSInteger videoSyntheticEncounteredFrames;
@property (nonatomic, assign) NSTimeInterval videoSyntheticPrevTime;
@property (nonatomic, assign) NSTimeInterval videoSyntheticElapsedTime;
@property (nonatomic, assign) NSTimeInterval videoSyntheticFrameDuration;
@property (nonatomic, assign) CFTimeInterval timeWhenRequestingKeyFrame;
@property (nonatomic, assign) CFTimeInterval periodUntilForceKeyFrame;
@property (nonatomic, assign) CMTime timeDelta;
@property (nonatomic, assign) BOOL sendFormatOnKeyFrames;

@end

static void encoderOutputCallback(void *outputCallbackRefCon,
								  void *sourceFrameRefCon,
								  OSStatus status,
								  VTEncodeInfoFlags infoFlags,
								  CMSampleBufferRef sampleBuffer)
{
	auto vcPointer = (__bridge KBVideoCompressorSession *) outputCallbackRefCon;
	if (!vcPointer) return;
	
	if (status != noErr)
	{
		VTCompressionSessionEndPass(vcPointer.sessionRef, nil, nil);
		vcPointer.sessionRef = nil;
		LOG_DEBUG(boost::format("encoderOutputCallback failed with error: %1%") % status);
		return;
	}
	
	if ([vcPointer delegate])
	{
		CMTime presentationTime = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
		CMTime duration = CMSampleBufferGetDuration(sampleBuffer);
		
		[vcPointer.delegate videoSampleEncoded: sampleBuffer
						  withPresentationTime: presentationTime
								   andDuration: duration];
	}
}

@implementation KBVideoCompressorSession

- (instancetype)initWithCodecType:(CMVideoCodecType)type andRtmpSettings: (KBRtmpSessionSettings *) settings
{
	self = [super init];
	
	if (self)
	{
		self.codecType = type;
		self.rtmpSettings = settings;
	}
	
	return self;
}

- (OSStatus) createCompressionSessionIfNeeded
{
	if (self.sessionRef) return noErr;
	
	NSDictionary *sourceImageBufferAttributes = @{};
	
#if defined TARGET_IOS
	NSDictionary *encoderSpecification = @{};
#else
	NSDictionary *encoderSpecification = @{kVTVideoEncoderSpecification_EnableHardwareAcceleratedVideoEncoder : @(true)};
#endif
	OSStatus err = noErr;
	
	err = VTCompressionSessionCreate(kCFAllocatorDefault,
									 [self.rtmpSettings width],
									 [self.rtmpSettings height],
									 CMVideoCodecType(self.codecType),
									 (__bridge CFDictionaryRef) encoderSpecification,
									 (__bridge CFDictionaryRef) sourceImageBufferAttributes,
									 kCFAllocatorDefault,
									 encoderOutputCallback,
									 (__bridge void *) self,
									 &_sessionRef);
	
	if (err != noErr || self.sessionRef == nil)
	{
		LOG_DEBUG(boost::format("Unable to create the VTCompressionSessionCreate: %1%") % err);
		return err;
	}
	
	[self videoSyntheticFrameMeterReset];
	[self setSendFormatOnKeyFrames: false];
	[self applySettingsToVideoCompressor];
	
	return err;
}

- (void)end
{
	if (!self.sessionRef) return;
	VTCompressionSessionInvalidate(_sessionRef);
	self.sessionRef = nil;
}

#pragma mark - Processing

- (BOOL) shouldSkipFrame:(NSTimeInterval) time
{
	BOOL drop = false;
	
	NSTimeInterval delta = time - self.videoSyntheticPrevTime;
	self.videoSyntheticPrevTime = time;
	
	if (self.videoSyntheticPrevTime == 0)
	{
		self.videoSyntheticEncounteredFrames += 1;
		return false;
	}
	
	self.videoSyntheticEncounteredFrames += 1;
	self.videoSyntheticElapsedTime += delta;
	
	NSTimeInterval frameDuration = self.videoSyntheticElapsedTime / self.videoSyntheticEncounteredFrames;
	
	if (frameDuration < self.videoSyntheticFrameDuration / 1.05)
	{
		self.videoSyntheticEncounteredFrames -= 1;
		drop = true;
	}
	
	if (self.videoSyntheticElapsedTime > 5)
	{
		NSTimeInterval oneFrameTime = self.videoSyntheticElapsedTime / self.videoSyntheticEncounteredFrames;
		NSInteger removeFrames = lround((self.videoSyntheticElapsedTime - 5) / oneFrameTime);
		
		self.videoSyntheticElapsedTime -= removeFrames * oneFrameTime;
		self.videoSyntheticEncounteredFrames -= removeFrames;
	}
	
	return drop;
}

- (CMSampleBufferRef _Nullable) checkSampleBufferDimension:(CMSampleBufferRef) sampleBuffer
{
	CVPixelBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
	
	if (imageBuffer != nil)
	{
		CGSize videoDimensionRect = CVImageBufferGetDisplaySize(imageBuffer);
		
		CMVideoDimensions dimensions = CMVideoDimensions {
			static_cast<int32_t>(videoDimensionRect.width),
			static_cast<int32_t>(videoDimensionRect.height)
		};
		
		CMVideoRect rect = CMVideoRect { 0, 0, dimensions.width, dimensions.height };
		CMSampleBufferRef finalSampleBuffer = sampleBuffer;
		const auto height = static_cast<int32_t>(rect.width / 16.0 * 9.0);
		
		if (rect.width < rect.height)
		{
			rect = CMVideoRect { 0, static_cast<int32_t>((rect.height - height) / 2.0), rect.width, height };
			finalSampleBuffer = [KBUtility sampleBuffer:sampleBuffer withVideoRect:rect andCopyPixels:true];
		}
		else if (static_cast<int32_t>(rect.width / 16.0 * 9.0) != rect.height)
		{
			rect = CMVideoRect { 0, 0, rect.width, height };
			finalSampleBuffer = [KBUtility sampleBuffer:sampleBuffer withVideoRect:rect andCopyPixels:true];
		}
		
		return finalSampleBuffer;
	}
	
	return sampleBuffer;
}

- (void) processSampleBuffer:(CMSampleBufferRef) sampleBuffer
{
	if (!sampleBuffer) return;
	
	CMFormatDescriptionRef format = CMSampleBufferGetFormatDescription(sampleBuffer);
	CMMediaType currentMediaType = CMFormatDescriptionGetMediaType(format);
	
	if (currentMediaType != kCMMediaType_Video)
	{
		LOG_DEBUG(boost::format("Wrong media type format: %1%") % currentMediaType);
		return;
	}
	
	CMSampleBufferRef finalSampleBuffer = [self checkSampleBufferDimension:sampleBuffer];
	
	if (finalSampleBuffer)
	{
		NSTimeInterval presentationTimeSeconds
			= CMTimeGetSeconds(CMSampleBufferGetPresentationTimeStamp(finalSampleBuffer));
		
		if (![self shouldSkipFrame: presentationTimeSeconds])
		{
			self.videoCapturedFrameIndex += 1;
			
			CFTimeInterval currentTime = CACurrentMediaTime();
			CFTimeInterval elapsed = currentTime - self.timeWhenRequestingKeyFrame;
			
			if (elapsed >= self.periodUntilForceKeyFrame)
			{
				lastTime = currentTime;
				
				[self encodeFrameWithSampleBuffer: finalSampleBuffer
									   frameIndex: self.videoCapturedFrameIndex
							 forceKeyFrameCounter: forceKeyFrameCounter];
			}
			else
			{
				int32_t noForce = 0;
				
				[self encodeFrameWithSampleBuffer: finalSampleBuffer
									   frameIndex: self.videoCapturedFrameIndex
							 forceKeyFrameCounter: noForce];
			}
		}
	}
}
			 
#pragma mark - Encoder

- (void) encodeFrameWithSampleBuffer: (CMSampleBufferRef) sampleBuffer
						  frameIndex: (NSInteger) frameIndex
				forceKeyFrameCounter: (int32_t) counter
{
	[self createCompressionSessionIfNeeded];
	NSDictionary *frameProperties = nil;
	
	if (OSAtomicDecrement32(&counter) >= 1)
	{
		frameProperties = @{
			(__bridge NSString *) kVTEncodeFrameOptionKey_ForceKeyFrame : (__bridge NSNumber *) kCFBooleanTrue
		};
	}
	
	VTEncodeInfoFlags infoFlags;
	CMTime time = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
	
	if (!CMTIME_IS_VALID(self.timeDelta))
	{
		self.timeDelta = time;
	}
	
	time.value -= self.timeDelta.value;
	CMTime duration = CMSampleBufferGetDuration(sampleBuffer);
	CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
	OSStatus err = noErr;

	uint8_t frameRefCon = static_cast<uint8_t>(frameIndex);
	err = VTCompressionSessionEncodeFrame(self.sessionRef,
										  pixelBuffer,
										  time,
										  duration,
										  (__bridge CFDictionaryRef)frameProperties,
										  &frameRefCon,
										  &infoFlags);
	
	if (err != noErr)
	{
		LOG_DEBUG(boost::format("Encoder failed with error: %1%") % err);
		[self end];
	}
}

- (void) applySettingsToVideoCompressor
{
	[self setVideoSyntheticFrameRate: self.rtmpSettings.frameRate];
	[self setVideoCompressorBitRate: (self.rtmpSettings.videoDataRate * 1024.)];
	
	if (self.rtmpSettings.keyFrameInterval > 0)
	{
		// if keyFrameInterval is set, we'll use that value, otherwise the compressor will choose the correct interval
		[self setVideoCompressorKeyFrameInterval: self.rtmpSettings.keyFrameInterval];
	}
	
	[self setVideoCompressorExpectedFrameRate: self.rtmpSettings.frameRate];
	[self setVideoCompressorIsRealTime: true];
	[self setVideoCompressorProfileLevel: kVTProfileLevel_H264_Baseline_3_1];
//	[self setVideoCompressorProfileLevel: kVTProfileLevel_H264_High_4_1];
	[self setVideoCompressorAllowFrameReordering: false];
}

- (void)setRtmpSettings:(KBRtmpSessionSettings *)rtmpSettings
{
	_rtmpSettings = rtmpSettings;
	[self applySettingsToVideoCompressor];
}

- (void) updateCompressorBitRate:(double) bitrate
{
	[self setVideoCompressorBitRate: bitrate];
}

- (double) resetCompressorBitRate
{
	const auto value = (self.rtmpSettings.videoDataRate * 1024.);
	[self setVideoCompressorBitRate: value];
	return value;
}

#pragma mark - Utility methods

- (void) reset
{
	[self videoSyntheticFrameMeterReset];
}

- (BOOL) isSessionAvailable
{
	return self.sessionRef != nil;
}

- (double)videoSyntheticFrameRate
{
	return 1.0 / self.videoSyntheticFrameDuration;
}

- (void) videoSyntheticFrameMeterReset
{
    self.timeDelta = kCMTimeInvalid;
	self.videoSyntheticElapsedTime = 0;
	self.videoSyntheticEncounteredFrames = 0;
	self.videoSyntheticPrevTime = 0;
}

- (CFTypeRef) getProperty:(CFStringRef) property
{
	CFTypeRef value;
	VTSessionCopyProperty(self.sessionRef, property, nil, &value);
	return value;
}

- (OSStatus) setPropertyWithKey:(CFStringRef) property andValue:(CFTypeRef) value
{
	OSStatus err = VTSessionSetProperty(self.sessionRef, property, value);
	return err;
}

- (void)setVideoSyntheticFrameRate:(double)videoSyntheticFrameRate
{
	self.videoSyntheticFrameDuration = 1./videoSyntheticFrameRate;
	[self videoSyntheticFrameMeterReset];
}

- (OSStatus) setVideoCompressorIsRealTime: (BOOL) value
{
	return [self setPropertyWithKey: kVTCompressionPropertyKey_RealTime
						   andValue: value ? kCFBooleanTrue : kCFBooleanFalse];
}

- (OSStatus) setVideoCompressorBitRate: (double) bitRate
{
	return [self setPropertyWithKey:kVTCompressionPropertyKey_AverageBitRate
						   andValue: (__bridge CFTypeRef) @(bitRate)];
}

- (OSStatus) setVideoCompressorKeyFrameInterval: (double) keyFrameInterval
{
	return [self setPropertyWithKey:kVTCompressionPropertyKey_MaxKeyFrameInterval
						   andValue: (__bridge CFTypeRef) @(keyFrameInterval)];
}

- (OSStatus) setVideoCompressorExpectedFrameRate: (double) frameRate
{
	return [self setPropertyWithKey:kVTCompressionPropertyKey_ExpectedFrameRate
						   andValue: (__bridge CFTypeRef) @(frameRate)];
}

- (OSStatus) setVideoCompressorAllowFrameReordering: (BOOL) value
{
	return [self setPropertyWithKey:kVTCompressionPropertyKey_AllowFrameReordering
						   andValue: (__bridge CFTypeRef) @(value)];
}

- (OSStatus) setVideoCompressorMaxFrameDelayCount: (double) value
{
	return [self setPropertyWithKey:kVTCompressionPropertyKey_MaxFrameDelayCount
						   andValue: (__bridge CFTypeRef) @(value)];
}

- (OSStatus) setVideoCompressorProfileLevel: (CFStringRef) profileLevel
{
	return [self setPropertyWithKey:kVTCompressionPropertyKey_ProfileLevel andValue:profileLevel];
}

- (OSStatus) setVideoCompressorH246EntropyMode: (NSString *) value
{
	return [self setPropertyWithKey:kVTCompressionPropertyKey_H264EntropyMode
						   andValue:(__bridge CFStringRef)value];
}

@end
