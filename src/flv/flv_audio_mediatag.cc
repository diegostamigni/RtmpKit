#include "../../private/flv/flv_audio_mediatag.h"
using namespace RtmpKit;

void FLVMediaAudioTag::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	FLVMediaTag::deserialize(begin, end);

	deserialize();
}

void FLVMediaAudioTag::deserialize()
{
	if (payload_.empty()) return;
	auto iter = payload_.cbegin();

	this->setSoundFormat(static_cast<FLVSoundFormatType>(((*iter) & 0xf0) >> 4));
	this->setSoundRate(static_cast<FLVSoundRateType>(((*iter) & 0x0C) >> 2));
	this->setSoundSampleSize(static_cast<FLVSoundSampleSizeType>(((*iter) & 0x02) >> 1));
	this->setSoundType(static_cast<FLVSoundType>(((*iter) & 0x01) >> 0));

	this->setAACPacketType(soundFormat() == FLVSoundFormatType::AAC
						   ? (static_cast<FLVAACPacketType>(*iter)) : FLVAACPacketType::Unknown);
}

const v8& FLVMediaAudioTag::serialize()
{
	return payload_;
}

void FLVMediaAudioTag::updateMedataData()
{
	const auto value = static_cast<u8>(soundFormat()) << 4
		| static_cast<u8>(soundRate()) << 2
		| static_cast<u8>(soundSampleSize()) << 1
		| static_cast<u8>(soundType()) << 0;
	
	payload_.insert(payload_.cbegin(), value);
	payload_.insert(payload_.cbegin() + 1, static_cast<u8>(1));
	
	payloadSize_ = static_cast<decltype(payloadSize_)>(payload_.size());
}
