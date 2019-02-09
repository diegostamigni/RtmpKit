#include "../../private/flv/flv_video_mediatag.h"
using namespace RtmpKit;

void FLVMediaVideoTag::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	FLVMediaTag::deserialize(begin, end);

	deserialize();
}

void FLVMediaVideoTag::deserialize()
{
	if (payload_.empty()) return;
	auto iter = payload_.cbegin();

	this->setCodecId(static_cast<FLVVideoCodecType>(((*iter) & 0x0f) >> 0));
	this->setFrameType(static_cast<FLVVideoFrameType>(((*iter) & 0xf0) >> 4));
}

const v8& FLVMediaVideoTag::serialize()
{
	return payload_;
}

void FLVMediaVideoTag::updateMedataData()
{
	const auto value = static_cast<u8>(frameType()) << 4 | static_cast<u8>(codecId());
	
	payload_.insert(payload_.cbegin(), value);
	payload_.insert(payload_.cbegin() + 1, static_cast<u8>(1));
	payload_.insert(payload_.cbegin() + 2, static_cast<u8>(0));
	payload_.insert(payload_.cbegin() + 3, static_cast<u8>(0));
	payload_.insert(payload_.cbegin() + 4, static_cast<u8>(0));
	
	payloadSize_ = static_cast<decltype(payloadSize_)>(payload_.size());
}
