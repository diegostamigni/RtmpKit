#include "../private/mediautils.h"
#include "../private/logger.h"

using namespace RtmpKit;
static const v8 naluHeader = { 0x00, 0x00, 0x00, 0x01 };

// Video stuff

VideoDescriptor MediaUtils::unpackVideoData(RtmpKit::v8::const_iterator& iter,
											RtmpKit::v8::const_iterator& end,
											RtmpKit::u32& timestamp,
											bool appendNaluHeader)
{
	VideoDescriptor descriptor;
	u8 codec, config = 0;
	
	codec = *iter;
	iter += 1;
	
	// RtmpKit::u8 isIntra = (codec >> 4) == 1;
	codec &= 0x0F;
	
	const auto acceptableCodec = (codec == 7);
	if (!acceptableCodec)
	{
		LOG_DEBUG(str(boost::format("Video unpacker found a wrong codec type: %1%")
		              % (int) codec).c_str());
		return descriptor;
	}
	
	config = *iter;
	iter += 1;
	
	const auto acceptableConfig = (config == 0 || config == 1 || config == 2);
	if (!acceptableConfig)
	{
		LOG_DEBUG(str(boost::format("Video unpacker found a wrong configuration byte: %1%")
		              % (int) config).c_str());
		return descriptor;
	}
	
	const auto delay = RtmpKit::fromBigEndian24({iter, iter + 3});
	timestamp += delay;
	iter += 3;
	
	if (config == 0)
	{
		// RtmpKit::u8 configurationVersion, avcProfileIndication, profileCompatibility, avcLevelIndication = 0;
		RtmpKit::u8 nalSizeLen, spsCount, ppsCount = 0;
		
		// configurationVersion = *iter;
		iter += 1;
		
		// avcProfileIndication = *iter;
		iter += 1;
		
		// profileCompatibility = *iter;
		iter += 1;
		
		// avcLevelIndication = *iter;
		iter += 1;
		
		nalSizeLen = *iter;
		iter += 1;
		
		const auto nalUnitSize = (nalSizeLen & 0x03) + 1;
		if (nalUnitSize != 4)
		{
			LOG_DEBUG(str(boost::format("Video unpacker found an unsupported NAL unit size: %1%")
			              % (int) nalUnitSize).c_str());
			return descriptor;
		}
		
		RtmpKit::u16 spsSize = 0;
		RtmpKit::u16 ppsSize = 0;
		
		spsCount = *iter;
		iter += 1;
		
		spsCount &= 0x1F;
		if (spsCount != 1)
		{
			LOG_DEBUG(str(boost::format("Video unpacker found an unsupported spsCount: %1%")
			              % (int) spsCount).c_str());
			return descriptor;
		}
		
		spsSize = RtmpKit::fromBigEndian16({iter, iter+sizeof(decltype(spsSize))});
		iter += sizeof(decltype(spsSize));

		auto sps = RtmpKit::v8{};
		sps.reserve(appendNaluHeader ? spsSize + naluHeader.size() : spsSize);

		if (appendNaluHeader)
		{
			std::copy(naluHeader.begin(), naluHeader.end(), std::back_inserter(sps));
		}

		std::move(iter, iter+spsSize, std::back_inserter(sps));
		iter += spsSize;
		
		ppsCount = *iter;
		iter += 1;
		
		if (ppsCount != 1)
		{
			LOG_DEBUG(str(boost::format("Video unpacker found an unsupported ppsCount: %1%")
			              % (int) ppsCount).c_str());
			return descriptor;
		}
		
		ppsSize = RtmpKit::fromBigEndian16({iter, iter+sizeof(decltype(ppsSize))});
		iter += sizeof(decltype(ppsSize));
		
		auto pps = RtmpKit::v8{};
		pps.reserve(appendNaluHeader ? ppsSize + naluHeader.size() : ppsSize);

		if (appendNaluHeader)
		{
			std::copy(naluHeader.begin(), naluHeader.end(), std::back_inserter(pps));
		}

		std::move(iter, iter+ppsSize, std::back_inserter(pps));
		iter += ppsSize;
		
		descriptor.setSPS(std::move(sps));
		descriptor.setPPS(std::move(pps));
		descriptor.setNalUnitSize(nalUnitSize);
		
		return descriptor;
	}
	
	return descriptor;
}

// Audio stuff

AudioDescriptor MediaUtils::unpackAudioData(RtmpKit::v8::const_iterator& iter,
											RtmpKit::v8::const_iterator& end)
{
	AudioDescriptor descriptor;
	const auto size = std::distance(iter, end);
	
	if (size < 4)
	{
		return descriptor;
	}
	
	const auto v0 = *iter;
	iter += 1;
	
	const auto v1 = *iter;
	iter += 1;
	
	// size -= 2;
	
	if (!(v0 == 0xAF || v0 == 0x2F))
	{
		LOG_DEBUG(str(boost::format("Audio unpacker found an unsupported v0 format: %1%")
		              % (int) v1).c_str());
		return descriptor;
	}
	
	if (!(v1 == 0 || v1 == 1 || v1 == 0xFF))
	{
		LOG_DEBUG(str(boost::format("Audio unpacker found an unsupported v1 format: %1%")
		          % (int) v1).c_str());
		return descriptor;
	}
	
	if (v1 == 0)
	{
		const auto v2 = *iter;
		iter += 1;
		
		const auto v3 = *iter;
		iter += 1;
		
		// const auto profile = (v2 >> 3) & 0x1F;
		descriptor.setChannelCount((v3 >> 3) & 0x0F);
		descriptor.setFrameLength((v3 & 0x04) ? 960 : 1024);
		
		const auto srIndex = ((v2 << 1) | (v3 >> 7)) & 0x0F;
		descriptor.setSampleRate(RtmpKit::MediaUtils::indexForSampleRate(srIndex));
	}
	
	return descriptor;
}

std::vector<int> MediaUtils::audioSampleRates()
{
	return {
		96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, -1, -1, -1
	};
}

int MediaUtils::indexForSampleRate(int sampleRate)
{
	static const auto sampleRates = MediaUtils::audioSampleRates();
	
	for (size_t i = 0; i < sampleRates.size(); i++)
	{
		if (abs(sampleRates[i] - sampleRate) < 1e-5)
		{
			return static_cast<int>(i);
		}
	}
	
	return -1;
}
