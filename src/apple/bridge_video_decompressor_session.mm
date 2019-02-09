#import "../../private/apple/KBVideoDecompressorSession.h"
#import "../../private/apple/KBUtility.h"

#include "../../private/logger.h"

#import <QuartzCore/QuartzCore.h>
#import <VideoToolbox/VideoToolbox.h>
#import <CoreImage/CoreImage.h>
#import <libkern/OSAtomic.h>

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
#endif

@interface KBVideoDecompressorSession()
{
}

@property (nonatomic, assign) VTDecompressionSessionRef sessionRef;
@property (nonatomic, assign) CMTimebaseRef timeBase;
@end

static void decoderOutputCallback(void *decompressionOutputRefCon,
								  void *sourceFrameRefCon,
								  OSStatus status,
								  VTDecodeInfoFlags infoFlags,
								  CVImageBufferRef imageBuffer,
								  CMTime presentationTimeStamp,
								  CMTime presentationDuration)
{
	auto vcPointer = (__bridge KBVideoDecompressorSession *) decompressionOutputRefCon;
	if (!vcPointer) return;
	
#if defined TARGET_IOS
	if ([UIApplication sharedApplication].applicationState != UIApplicationStateActive)
	{
		return;
	}
#endif
	
	if (status != noErr)
	{
		LOG_DEBUG(boost::format("decoderOutputCallback failed with error: %1%") % status);
		return;
	}
	
	if (imageBuffer)
    {
        if(vcPointer.delegate &&
           [vcPointer.delegate respondsToSelector:@selector(videoSampleDecoded:
                                                            withPresentationTime:
                                                            andDuration:
                                                            withTimebase:)])
        {
            CIImage *image = [CIImage imageWithCVPixelBuffer:imageBuffer options:@{kCIImageColorSpace: [NSNull new]}];
            [vcPointer.delegate videoSampleDecoded:image
                              withPresentationTime:presentationTimeStamp
                                       andDuration:presentationDuration
                                      withTimebase:vcPointer.timeBase];
        }
	}
}

@implementation KBVideoDecompressorSession

- (instancetype)initWithRtmpSettings: (KBRtmpSessionSettings *) settings
{
	self = [self init];
	
	if (self)
	{
		self.rtmpSettings = settings;
	}
	
	return self;
}

- (OSStatus) createDecompressionSessionIfNeeded: (CMSampleBufferRef) sampleBuffer
						  withFormatDescription: (CMFormatDescriptionRef) formatDescription
{
	if (!sampleBuffer) return -1;
	if (self.sessionRef) return noErr;
	
	OSStatus err = noErr;
	
	if (!formatDescription)
	{
		formatDescription = CMSampleBufferGetFormatDescription(sampleBuffer);
	}
	
	CMVideoDimensions dimesions = CMVideoFormatDescriptionGetDimensions(formatDescription);
	
	NSDictionary *videoDecoderSpecification = @{
		(__bridge NSString *) kVTDecompressionPropertyKey_RealTime : @(true),
		(__bridge NSString *) kVTDecompressionPropertyKey_ThreadCount : @(2)
	};
	
	NSDictionary *destinationImageBufferAttributes = @{
		(__bridge NSString *) kCVPixelBufferWidthKey : @(dimesions.width),
		(__bridge NSString *) kCVPixelBufferHeightKey : @(dimesions.height),
		(__bridge NSString *) kCVPixelBufferOpenGLESCompatibilityKey : @(true),
		(__bridge NSString *) kCVPixelBufferPixelFormatTypeKey : [NSNumber numberWithUnsignedInteger:kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange]
	};
	
	CMTime outputPresentationTime = CMSampleBufferGetOutputPresentationTimeStamp(sampleBuffer);
	self.timeBase = [KBUtility timebaseWithCMTime:outputPresentationTime];
	
	VTDecompressionOutputCallbackRecord callback = { decoderOutputCallback, (__bridge void*) self };
	
	err = VTDecompressionSessionCreate(nil,
									   formatDescription,
									   (__bridge CFDictionaryRef) videoDecoderSpecification,
									   (__bridge CFDictionaryRef) destinationImageBufferAttributes,
									   &callback,
									   &_sessionRef);
	
	if (err != noErr || self.sessionRef == nil)
	{
		LOG_DEBUG(boost::format("Unable to create the VTDecompressionSessionCreate: %1%") % err);
		return err;
	}
	
	return err;
}

- (void) end
{
	[self reset];
}

- (void) reset
{
	if (!self.sessionRef) return;
	VTDecompressionSessionInvalidate(self.sessionRef);
	self.sessionRef = nil;
}

#pragma mark - Decoder

- (void) decodeFrameWithSampleBuffer: (CMSampleBufferRef) sampleBuffer
{
#if defined TARGET_IOS
	if ([UIApplication sharedApplication].applicationState != UIApplicationStateActive)
	{
		return;
	}
#endif
	
	[self decodeFrameWithSampleBuffer:sampleBuffer andFormatDescription:nil];
}

- (void) decodeFrameWithSampleBuffer: (CMSampleBufferRef) sampleBuffer
				andFormatDescription: (CMFormatDescriptionRef) formatDescription
{
	const auto release = [&sampleBuffer]()
	{
		CMSampleBufferInvalidate(sampleBuffer);
		CFRelease(sampleBuffer);
	};
	
	if (!sampleBuffer) return;
	
#if defined TARGET_IOS
	if ([UIApplication sharedApplication].applicationState != UIApplicationStateActive)
	{
		release();
		return;
	}
#endif
	
	[self createDecompressionSessionIfNeeded: sampleBuffer
					   withFormatDescription: formatDescription];
	
	if (!self.sessionRef)
	{
		release();
		return;
	}
	
	VTDecodeInfoFlags outFlags;
	VTDecodeFrameFlags flags =
	kVTDecodeFrame_EnableAsynchronousDecompression |
	kVTDecodeFrame_EnableTemporalProcessing |
	kVTDecodeFrame_1xRealTimePlayback;
	
	OSStatus err = VTDecompressionSessionDecodeFrame(_sessionRef,
													 sampleBuffer,
													 flags,
													 nil,
													 &outFlags);
	
	if (err != noErr)
	{
		LOG_DEBUG(boost::format("Decoder failed with error: %1%") % err);
		[self reset];
	}
	
	release();
}

- (void)setRtmpSettings:(KBRtmpSessionSettings *)rtmpSettings
{
	_rtmpSettings = rtmpSettings;
}

@end
