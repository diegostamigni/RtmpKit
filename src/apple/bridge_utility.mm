#import "../../private/apple/KBUtility.h"
#import "../../private/logger.h"
#import "../../private/mediautils.h"

#if defined TARGET_IOS
	#import <UIKit/UIKit.h>
#endif

@implementation KBUtility

+ (CGSize) defaultOffsetForOnStageInBounds: (CGRect) bounds
{
	return CGSizeMake((bounds.size.width - (bounds.size.width / 4)), 0);
}

+ (NSData *_Nullable) h264_paramDataWithFormatDescription: (CMFormatDescriptionRef _Nonnull) formatDescription
												  atIndex: (size_t) index
{
	size_t paramCount = 0;
	const uint8_t *param = nil;
	size_t paramSize = 0;
	
	OSStatus err = CMVideoFormatDescriptionGetH264ParameterSetAtIndex(formatDescription,
																	  index,
																	  &param,
																	  &paramSize,
																	  &paramCount,
																	  nil);
	
	if (err != noErr)
	{
        LOG_DEBUG(
			boost::format("An error occurred while getting the h264_paramDataWithFormatDescription: %1%") % (int) err);
		return nil;
	}
	
	return [[NSData alloc] initWithBytes: param length: paramSize];
}

+ (BOOL) isKeyFrame:(CMSampleBufferRef)sampleBuffer
{
	NSArray *attachments = (__bridge NSArray *) CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, false);
	if (attachments == nil) return false;
	
	for (NSDictionary<NSString*, NSNumber*> *attachment in attachments)
	{
		NSNumber *dependsOnOthers = attachment[(__bridge NSString *)kCMSampleAttachmentKey_DependsOnOthers];
		
		if (![dependsOnOthers boolValue])
		{
			return true;
		}
	}
	
	return false;
}

+ (uint8_t *) bufferToRaw: (CMSampleBufferRef) source
{
	if (!source) return nil;
	CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(source);
	
	uint8_t *data = nullptr;
	size_t size = 0;
	
	OSStatus err = CMBlockBufferGetDataPointer(blockBuffer, 0, nil, &size, (char **) &data);
	
	if (err != noErr)
	{
		LOG_DEBUG(boost::format("An error occurred while converting the buffer to an u8 ptr: %1%") % err);
		return nullptr;
	}
	
	return data;
}

+ (NSData *)bufferToData:(CMSampleBufferRef)source
{
	if (!source) return nil;
	
	const auto lenght = [KBUtility bufferSize:source];
	uint8_t *data = [KBUtility bufferToRaw:source];
	
	if (data)
	{
		return [[NSData alloc] initWithBytes: data length: lenght];
	}
	else
	{
		return nil;
	}
}

+ (size_t) bufferSize: (CMSampleBufferRef) source
{
	if (!source) return 0;
	
	const auto result = CMSampleBufferGetTotalSampleSize(source);
	return result;
}

+ (int32_t) h264_nalUnitHeaderLengthWithSampleBuffer: (CMSampleBufferRef) sampleBuffer
{
	if (!sampleBuffer) return 0;
	return [KBUtility h264_nalUnitHeaderLengthWithFormatDescription: CMSampleBufferGetFormatDescription(sampleBuffer)];
}

+ (int32_t) h264_nalUnitHeaderLengthWithFormatDescription: (CMFormatDescriptionRef) formatDescription
{
	int32_t nalUnitHeaderLength = 0;
	if (!formatDescription) return nalUnitHeaderLength;
	
	OSStatus err = CMVideoFormatDescriptionGetH264ParameterSetAtIndex(formatDescription,
																	  0,
																	  nil,
																	  nil,
																	  nil,
																	  &nalUnitHeaderLength);
	
	if (err != noErr)
	{
		LOG_DEBUG(boost::format("An error occurred while getting the h264_nalUnitHeaderLength: %1%") % err);
		return 0;
	}
	
	return nalUnitHeaderLength;
}

+ (NSData *) h264_spsDataWithSampleBuffer: (CMSampleBufferRef _Nullable) sampleBuffer
{
	if (!sampleBuffer) return nil;
	return [KBUtility h264_spsDataWithFormatDescription: CMSampleBufferGetFormatDescription(sampleBuffer)];
}

+ (NSData *) h264_spsDataWithFormatDescription: (CMFormatDescriptionRef _Nullable) formatDescription
{
	if (!formatDescription) return nil;
	return [KBUtility h264_paramDataWithFormatDescription:formatDescription atIndex:0];
}

+ (NSData *) h264_ppsDataWithSampleBuffer: (CMSampleBufferRef _Nullable) sampleBuffer
{
	if (!sampleBuffer) return nil;
	return [KBUtility h264_ppsDataWithFormatDescription: CMSampleBufferGetFormatDescription(sampleBuffer)];
}

+ (NSData *_Nullable) h264_ppsDataWithFormatDescription: (CMFormatDescriptionRef _Nullable) formatDescription
{
	if (!formatDescription) return nil;
	return [KBUtility h264_paramDataWithFormatDescription:formatDescription atIndex:1];
}

+ (CMTimebaseRef) timebaseWithCMTime: (CMTime) cmTime
{
	CMTimebaseRef timebase;
	
	CMTimebaseCreateWithMasterClock(kCFAllocatorDefault, CMClockGetHostTimeClock(), &timebase);
	CMTimebaseSetTime(timebase, cmTime);
	CMTimebaseSetRate(timebase, 1);
	
	return timebase;
}

+ (CMBlockBufferRef) blockBufferWithBytes: (const uint8_t *) data
									 size: (size_t) size
								timestamp: (CMTime) timestamp
{
	CMBlockBufferRef block;
	OSStatus err = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorSystemDefault,
													  nil,
													  size,
													  kCFAllocatorDefault,
													  nil,
													  0,
													  size,
													  kCMBlockBufferAssureMemoryNowFlag,
													  &block);
	
	if (err != noErr)
	{
		LOG_DEBUG(boost::format("An error occurred while creating the CMBlockBufferCreateWithMemoryBlock: %1%") % err);
		return nil;
	}
	
	err = CMBlockBufferReplaceDataBytes(data, block, 0, size);
	
	if (err != noErr)
	{
		LOG_DEBUG(boost::format("An error occurred while CMBlockBufferReplaceDataBytes: %1%") % err);
		return nil;
	}
	
	return block;
}

+ (CMSampleBufferRef) sampleBufferWithBytes: (const uint8_t *) data
									   size: (size_t) size
						  formatDescription: (CMFormatDescriptionRef) formatDescription
								  timestamp: (CMTime) timestamp
{
	CMBlockBufferRef block = [KBUtility blockBufferWithBytes:data
														size:size
												   timestamp:timestamp];
	
	if (block)
	{
		size_t dataSize = CMBlockBufferGetDataLength(block);
		
		CMSampleBufferRef sampleBuffer;
		CMSampleTimingInfo sampleTimingInfo = CMSampleTimingInfo { CMTimeMake(1, 24), timestamp, timestamp };
		
		OSStatus err = CMSampleBufferCreateReady(kCFAllocatorSystemDefault,
												 block,
												 formatDescription,
												 1,
												 1,
												 &sampleTimingInfo,
												 1,
												 &dataSize,
												 &sampleBuffer);
		
		if (err != noErr)
		{
			LOG_DEBUG(boost::format("An error occurred while CMSampleBufferCreateReady: %1%") % err);
			return nil;
		}
		
		CFRelease(block);
		return sampleBuffer;
	}
	
	return nil;
}

+ (CMFormatDescriptionRef) formatDescriptionFromSequenceParameterSet: (const uint8_t *) sps
														  withLength: (size_t) spsLength
											  andPictureParameterSet: (const uint8_t *) pps
														  withLength: (size_t) ppsLength
											 withNalUnitHeaderLength: (int32_t) nalUnitHeaderLength
{
	const uint8_t* props[2] = {sps, pps};
	const size_t sizes[2] = {spsLength, ppsLength};
	
	CMFormatDescriptionRef description;
	OSStatus err = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
																	   2,
																	   props,
																	   sizes,
																	   nalUnitHeaderLength,
																	   &description);
	
	if (err != noErr)
	{
		LOG_DEBUG(
			boost::format("An error occurred while CMVideoFormatDescriptionCreateFromH264ParameterSets: %1%") % err);
		return nil;
	}
	
	return description;
}

+ (CMSampleBufferRef) sampleBuffer: (CMSampleBufferRef) sBuffer
					 withVideoRect: (CMVideoRect) rect
					 andCopyPixels: (BOOL) copy
{
	if (!sBuffer) return nil;
	
	CMSampleBufferRef otherSB = sBuffer;
	CVPixelBufferRef otherPB = CMSampleBufferGetImageBuffer(sBuffer);
	CVPixelBufferRef pb = [KBUtility pixelBuffer:otherPB withVideoRect:rect andCopyPixels:copy];
	
	CMSampleTimingInfo sampleTimingInfo = CMSampleTimingInfo();
	CMSampleBufferGetSampleTimingInfo(otherSB, 0, &sampleTimingInfo);
	
	CMSampleBufferRef sampleBuffer = nil;
	CMFormatDescriptionRef formatDescription = CMSampleBufferGetFormatDescription(sampleBuffer);
	CMSampleBufferCreateForImageBuffer(nil, pb, true, nil, nil, formatDescription, &sampleTimingInfo, &sampleBuffer);
	
	return sampleBuffer;
}

+ (CVPixelBufferRef) pixelBuffer: (CVPixelBufferRef) pBuffer
				   withVideoRect: (CMVideoRect) rect
				   andCopyPixels: (BOOL) copy
{
	if (!pBuffer) return nil;
	
	CVPixelBufferRef p = pBuffer;
	CMVideoRect r = rect;
	CVPixelBufferRef pixelBuffer = nil;
	
	r = CMVideoRect { (r.x / 2) * 2, (r.y / 2) * 2, (r.width / 2) * 2, (r.height / 2) * 2 };
	
	auto x = r.x;
	auto y = r.y;
	auto w = r.width;
	auto h = r.height;
	auto row = CVPixelBufferGetBytesPerRow(p);
	auto dataSize = CVPixelBufferGetDataSize(p);
	auto format = CVPixelBufferGetPixelFormatType(p);
	
	
	CVPixelBufferLockBaseAddress(p, kCVPixelBufferLock_ReadOnly);
	uint8_t *base = static_cast<uint8_t *>(CVPixelBufferGetBaseAddress(p));
	
	OSStatus err = noErr;
	
	NSDictionary *attrs = @{
		(__bridge NSString *) kCVPixelBufferIOSurfacePropertiesKey : @{},
		(__bridge NSString *) kCVPixelBufferBytesPerRowAlignmentKey : @(row)
	};
	
	if (CVPixelBufferIsPlanar(p))
	{
		auto base0 = static_cast<uint8_t *>(CVPixelBufferGetBaseAddressOfPlane(p, 0));
		auto base1 = static_cast<uint8_t *>(CVPixelBufferGetBaseAddressOfPlane(p, 1));
		
		auto w1 = w / 2;
		auto h1 = h / 2;
		auto row0 = CVPixelBufferGetBytesPerRowOfPlane(p, 0);
		auto row1 = CVPixelBufferGetBytesPerRowOfPlane(p, 1);
		
		base0 += y * row0 + x;
		base1 += (y / 2) * row1 + (x / 2) * 2;
		
		if (!copy)
		{
			void* bases[2] = { static_cast<void *>(base0), static_cast<void *>(base1) };
			size_t rows[2] = { row0, row1 };
			size_t ws[2] = { static_cast<size_t>(w), static_cast<size_t>(w1) };
			size_t hs[2] = { static_cast<size_t>(h), static_cast<size_t>(h1) };
			
			err = CVPixelBufferCreateWithPlanarBytes(kCFAllocatorDefault,
													 w,
													 h,
													 format,
													 base,
													 dataSize,
													 CVPixelBufferGetPlaneCount(p),
													 bases,
													 ws,
													 hs,
													 rows,
													 nil,
													 nil,
													 (__bridge CFDictionaryRef) attrs,
													 &pixelBuffer);
			
			if (err != noErr)
			{
				LOG_DEBUG(boost::format("An error occurred while CVPixelBufferCreateWithPlanarBytes: %1%") % err);
				return nil;
			}
		}
		else
		{
			err = CVPixelBufferCreate(kCFAllocatorDefault,
									  w,
									  h,
									  format,
									  (__bridge CFDictionaryRef) attrs,
									  &pixelBuffer);
			
			if (err != noErr)
			{
				LOG_DEBUG(boost::format("An error occurred while CVPixelBufferCreate: %1%") % err);
				return nil;
			}
		}
		
		if (pixelBuffer && copy)
		{
			CVPixelBufferLockBaseAddress(pixelBuffer, CVPixelBufferLockFlags { 0 });
			memcpy(CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0), base0, row0 * h);
			memcpy(CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1), base1, row1 * h1);
			CVPixelBufferUnlockBaseAddress(pixelBuffer, CVPixelBufferLockFlags { 0 });
		}
		
		CVPixelBufferUnlockBaseAddress(p, kCVPixelBufferLock_ReadOnly);
		CVBufferPropagateAttachments(p, pixelBuffer);
		
		return pixelBuffer;
	}
	else
	{
		auto bpp = row / CVPixelBufferGetWidth(p);
		base += y * row + x * bpp;

		CVPixelBufferCreateWithBytes(kCFAllocatorDefault,
									 w,
									 h,
									 format,
									 base,
									 row,
									 nil,
									 nil,
									 (__bridge CFDictionaryRef) attrs,
									 &pixelBuffer);

		if (copy)
		{
			CVPixelBufferLockBaseAddress(pBuffer, CVPixelBufferLockFlags { 0 });
			memcpy(CVPixelBufferGetBaseAddress(pBuffer), base, row * h);
			CVPixelBufferUnlockBaseAddress(pBuffer, CVPixelBufferLockFlags { 0 });
		}
		
		CVPixelBufferUnlockBaseAddress(p, kCVPixelBufferLock_ReadOnly);
		CVBufferPropagateAttachments(p, pixelBuffer);
		
		return pixelBuffer;
	}
}

+ (CGRect) fixDrawingRectIfNeeded: (CGRect) rect inBounds: (CGRect) bounds
{
	const auto offset = [KBUtility defaultOffsetForOnStageInBounds:bounds];
	return [KBUtility fixDrawingRectIfNeeded:rect inBounds:bounds withOffset:offset];
}

+ (CGRect) fixDrawingRectIfNeeded: (CGRect) rect inBounds: (CGRect) bounds withOffset: (CGSize) offset
{
	auto dst = rect;
	
#if defined TARGET_IOS
	const auto orientation = [[UIApplication sharedApplication] statusBarOrientation];
	
	if (orientation == UIInterfaceOrientationLandscapeLeft || orientation == UIInterfaceOrientationLandscapeRight)
	{
		dst.origin.x = offset.width;
		dst.origin.y = offset.height;
	}
#endif
	
	return dst;
}

+ (RtmpKit::v8) buildParametersVideoPacketWithSPS:(NSData *) sps andPPS:(NSData *) pps andNalSizeLen:(int32_t) nl
{
	auto data = RtmpKit::v8{};
	
	auto spsData = static_cast<const uint8_t*>([sps bytes]);
	const auto spsSize = [sps length];
	
	auto ppsData = static_cast<const uint8_t*>([pps bytes]);
	const auto ppsSize = [pps length];
	
	// 6 bits reserved + 2 bits nal size length - 1
	RtmpKit::u8 nalSizeLen = 0xFC | (nl - 1);
	
	// 3 bits reserved + 5 bits number of sps
	RtmpKit::u8 spsCount = 0xE0 | 1;
	
	RtmpKit::u8 codec = 0x17;
	RtmpKit::u8 notConfig = 0;
	RtmpKit::u32 delay = 0;
	RtmpKit::u8 configurationVersion = 1;
	RtmpKit::u8 avcProfileIndication = spsData[1];
	RtmpKit::u8 profileCompatibility = spsData[2];
	RtmpKit::u8 avcLevelIndication = spsData[3];
	
	data.push_back(codec);
	data.push_back(notConfig);
	
	auto d = RtmpKit::toBigEndian24(delay);
	std::move(d.begin(), d.end(), std::back_inserter(data));
	
	data.push_back(configurationVersion);
	data.push_back(avcProfileIndication);
	data.push_back(profileCompatibility);
	data.push_back(avcLevelIndication);
	
	data.push_back(nalSizeLen);
	data.push_back(spsCount);
	
	{
		auto dSpsSize = RtmpKit::toBigEndian16(spsSize);
		std::move(dSpsSize.begin(), dSpsSize.end(), std::back_inserter(data));
		
		auto dSps = RtmpKit::v8(spsData, spsData + spsSize);
		std::move(dSps.begin(), dSps.end(), std::back_inserter(data));
	}
	
	uint8_t ppsCount = 1;
	data.push_back(ppsCount);
	
	{
		auto dPpsSize = RtmpKit::toBigEndian16(ppsSize);
		std::move(dPpsSize.begin(), dPpsSize.end(), std::back_inserter(data));
		
		auto dPps = RtmpKit::v8(ppsData, ppsData + ppsSize);
		std::move(dPps.begin(), dPps.end(), std::back_inserter(data));
	}
	
	return data;
}

+ (RtmpKit::v8) buildParametersAudioPacket
{
	// TODO (diegostamigni): grab this values from the settings
	auto result = RtmpKit::v8{};
	uint8_t profile = 2;
	int channelCount = 1;
	int frameLength = 1024;
	
	int index = RtmpKit::MediaUtils::indexForSampleRate(kSampleRate);
	uint8_t srIndex = index >= 0 ? index : 4;
	
	result.push_back(0xAF);
	result.push_back(0);
	result.push_back((profile << 3) | (srIndex >> 1));
	result.push_back((srIndex << 7) | (channelCount << 3) | ((frameLength == 960) ? 0x04 : 0x00));
	
	return result;
}

+ (RtmpKit::v8) buildParametersVideoPacketCompressedSample:(CMSampleBufferRef _Nonnull)compressedSample
{
	const auto nalUnitSize = [KBUtility h264_nalUnitHeaderLengthWithSampleBuffer:compressedSample];
	auto *spsData = [KBUtility h264_spsDataWithSampleBuffer:compressedSample];
	auto *ppsData = [KBUtility h264_ppsDataWithSampleBuffer:compressedSample];
	
	if (spsData && ppsData)
	{
		return [KBUtility buildParametersVideoPacketWithSPS:spsData
													 andPPS:ppsData
											  andNalSizeLen:nalUnitSize];
	}
	
	return {};
}

+ (std::pair<RtmpKit::v8::const_iterator, CMFormatDescriptionRef>)
	unpackVideoSpsAndPps:(const RtmpKit::v8&) payload
		   withTimestamp: (RtmpKit::u32&) timestamp
{
	auto begin = payload.cbegin(), end = payload.cend();
	const auto descriptor = RtmpKit::MediaUtils::unpackVideoData(begin, end, timestamp);

	if (descriptor.isValid())
	{
		const auto &sps = descriptor.sps();
		const auto &pps = descriptor.pps();
		
		const auto fd = [KBUtility formatDescriptionFromSequenceParameterSet:sps.data()
																  withLength:sps.size()
													  andPictureParameterSet:pps.data()
																  withLength:pps.size()
													 withNalUnitHeaderLength:descriptor.nalUnitSize()];
		
		return { begin, fd };
	}
	
	return { begin, nullptr };
}

+ (RtmpKit::v8::const_iterator) unpackAudioParams:(const RtmpKit::v8&) payload
								 newAudioFormat: (void(^)(AudioStreamBasicDescription audioFormat)) block
{
	auto begin = payload.cbegin(), end = payload.cend();
	const auto descriptor = RtmpKit::MediaUtils::unpackAudioData(begin, end);
	
	if (descriptor.isValid())
	{
		AudioStreamBasicDescription audioFormat;
		memset(&audioFormat, 0, sizeof(audioFormat));
		
		audioFormat.mChannelsPerFrame = descriptor.channelCount();
		audioFormat.mFormatID = kAudioFormatMPEG4AAC;
		audioFormat.mSampleRate = descriptor.sampleRate() > 0 ? descriptor.sampleRate() : kSampleRate;
		audioFormat.mFormatFlags = kMPEG4Object_AAC_LC;
		audioFormat.mBytesPerFrame = 0;
		audioFormat.mBytesPerPacket = 0;
		audioFormat.mFramesPerPacket = descriptor.frameLength();
		
		block(audioFormat);
	}
	
	return begin;
}

+ (void) unpackVideoPacket: (RtmpKit::RtmpVideoMessage &&) packet
			  withTimebase: (RtmpKit::u32&) timebase
	  andFormatDescription: (CMFormatDescriptionRef &) formatDescription
				completion: (void(^)(CMSampleBufferRef sampleBuffer)) block
{
	if (!packet.hasExtendedTimestamp())
	{
		timebase = packet.chunkStreamType() == RtmpKit::RtmpChunkStreamType::Type0
			? packet.timestamp()
			: timebase + packet.timestamp();
	}

	const auto &payload = packet.data();
	if (payload.empty()) return;

	const auto p = [KBUtility unpackVideoSpsAndPps:payload withTimestamp:timebase];
	
	if (p.second)
	{
		if (formatDescription)
		{
			CFRelease(formatDescription);
		}
		
		formatDescription = p.second;
	}
	
	if (!formatDescription) return;
	if (std::distance(p.first, payload.cend()) < 1) return;
	
	auto videoData = RtmpKit::v8{};
	std::move(p.first, payload.cend(), std::back_inserter(videoData));
	
	if (CMSampleBufferRef sampleBuffer = [KBUtility sampleBufferWithBytes:videoData.data()
																	 size:videoData.size()
														formatDescription:formatDescription
																timestamp:CMTimeMake(timebase, 1000)])
	{
		block(sampleBuffer);
	}
}

+ (void) unpackAudioPacket: (RtmpKit::RtmpAudioMessage &&) packet
			  withTimebase: (RtmpKit::u32&) timebase
			newAudioFormat: (void(^)(AudioStreamBasicDescription audioFormat)) newAudioFormatBlock
				completion: (void(^)(RtmpKit::v8&& data, CMTime timestamp)) completionBlock
{
	if (!packet.hasExtendedTimestamp())
	{
		timebase = packet.chunkStreamType() == RtmpKit::RtmpChunkStreamType::Type0
			? packet.timestamp()
			: timebase + packet.timestamp();
	}
	
	const auto &payload = packet.data();
	if (payload.empty()) return;
	
	const auto it = [KBUtility unpackAudioParams: payload
								  newAudioFormat:^(AudioStreamBasicDescription audioFormat)
	{
		newAudioFormatBlock(audioFormat);
	}];
	
	if (std::distance(it, payload.cend()) < 1) return;
	
	auto audioData = RtmpKit::v8{};
	std::move(it, payload.cend(), std::back_inserter(audioData));
	
	completionBlock(std::move(audioData), CMTimeMake(timebase, 1000));
}

@end
