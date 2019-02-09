//
//  KBUtility.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 13/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>

#define kOutputBus 0
#define kInputBus 1

#define kSampleRate __kSampleRateHigh
#define __kDefaultSampleMid 44100
#define __kSampleRateHigh 48000

#define kSampleSize 16
#define kFrameLenght 1024
#define kChannelCount 1
#define kBitRate 60000
#define kAudioSilenceErr 'Slce'
#define kAudioVideoAllowedDelay 200

#if defined __cplusplus

	#include "../../private/utils.h"
	#include "../../private/rtmp/message/rtmp_audio_message.h"
	#include "../../private/rtmp/message/rtmp_video_message.h"

	typedef struct CMVideoRect
	{
		int32_t x, y, width, height;
	} CMVideoRect;

#endif

@interface KBUtility : NSObject

+ (BOOL) isKeyFrame:(CMSampleBufferRef _Nullable)sampleBuffer;

+ (NSData *_Nullable) bufferToData: (CMSampleBufferRef _Nullable) source;
+ (uint8_t *_Nullable) bufferToRaw: (CMSampleBufferRef _Nullable) source;

+ (size_t) bufferSize: (CMSampleBufferRef _Nullable) source;

+ (int32_t) h264_nalUnitHeaderLengthWithSampleBuffer: (CMSampleBufferRef _Nullable) sampleBuffer;
+ (int32_t) h264_nalUnitHeaderLengthWithFormatDescription: (CMFormatDescriptionRef _Nullable) formatDescription;

+ (NSData *_Nullable) h264_spsDataWithSampleBuffer: (CMSampleBufferRef _Nullable) sampleBuffer;
+ (NSData *_Nullable) h264_spsDataWithFormatDescription: (CMFormatDescriptionRef _Nullable) formatDescription;

+ (NSData *_Nullable) h264_ppsDataWithSampleBuffer: (CMSampleBufferRef _Nullable) sampleBuffer;
+ (NSData *_Nullable) h264_ppsDataWithFormatDescription: (CMFormatDescriptionRef _Nullable) formatDescription;

+ (CMTimebaseRef _Nullable) timebaseWithCMTime: (CMTime) cmTime;

+ (CMBlockBufferRef _Nullable) blockBufferWithBytes: (const uint8_t *_Nonnull) data
											   size: (size_t) size
										  timestamp: (CMTime) timestamp;

+ (CMSampleBufferRef _Nullable) sampleBufferWithBytes: (const uint8_t *_Nonnull) data
												 size: (size_t) size
									formatDescription: (CMFormatDescriptionRef _Nonnull) formatDescription
											timestamp: (CMTime) timestamp;

+ (CMFormatDescriptionRef _Nullable) formatDescriptionFromSequenceParameterSet: (const uint8_t *_Nonnull) sps
																	withLength: (size_t) spsLength
														andPictureParameterSet: (const uint8_t *_Nonnull) pps
																	withLength: (size_t) ppsLength
													   withNalUnitHeaderLength: (int32_t) nalUnitHeaderLength;

+ (CMSampleBufferRef _Nullable) sampleBuffer: (CMSampleBufferRef _Nullable) sampleBuffer
							   withVideoRect: (CMVideoRect) rect
							   andCopyPixels: (BOOL) copy;

+ (CVPixelBufferRef _Nullable) pixelBuffer: (CVPixelBufferRef _Nullable) pixelBuffer
							 withVideoRect: (CMVideoRect) rect
							 andCopyPixels: (BOOL) copy;

+ (CGRect) fixDrawingRectIfNeeded: (CGRect) rect inBounds: (CGRect) bounds;

+ (CGRect) fixDrawingRectIfNeeded: (CGRect) rect inBounds: (CGRect) bounds withOffset: (CGSize) offset;


#if defined __cplusplus

+ (RtmpKit::v8) buildParametersAudioPacket;

+ (RtmpKit::v8) buildParametersVideoPacketWithSPS:(NSData *_Nonnull) sps
										 andPPS:(NSData *_Nonnull) pps
								  andNalSizeLen:(int32_t) nalSizeLen;

+ (RtmpKit::v8) buildParametersVideoPacketCompressedSample:(CMSampleBufferRef _Nonnull)compressedSample;

+ (void) unpackVideoPacket: (RtmpKit::RtmpVideoMessage &&) packet
			  withTimebase: (RtmpKit::u32&) timebase
	  andFormatDescription: (CMFormatDescriptionRef _Nullable &) formatDescription
				completion: (void(^_Nonnull)(CMSampleBufferRef _Nullable sampleBuffer)) block;

+ (void) unpackAudioPacket: (RtmpKit::RtmpAudioMessage &&) packet
			  withTimebase: (RtmpKit::u32&) timebase
			newAudioFormat: (void(^_Nonnull)(AudioStreamBasicDescription audioFormat)) newAudioFormatBlock
				completion: (void(^_Nonnull)(RtmpKit::v8&& data, CMTime timestamp)) completionBlock;

#endif

@end
