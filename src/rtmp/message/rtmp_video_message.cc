#include "../../../private/rtmp/message/rtmp_video_message.h"
using namespace RtmpKit;

RtmpVideoMessage::RtmpVideoMessage(u8 chunkStreamId, v8&& data)
{
	setChunkStreamFmt(chunkStreamId);
	setChunkStreamType(RtmpChunkStreamType::Type3);
	setType(RtmpMessageType::VideoMessage);
	setPayloadSize(static_cast<u24>(data.size()));

	videoTag_.setPayload(std::forward<v8>(data));
	videoTag_.deserialize();
}

RtmpVideoMessage::RtmpVideoMessage(FLVMediaVideoTag&& tag)
	: videoTag_(std::forward<FLVMediaVideoTag>(tag))
{
	setChunkStreamFmt(static_cast<u8>(RtmpChunkStreamFmt::VideoMessage));
	setTimestamp(tag.timestamp());
	setPayloadSize(tag.payloadSize());
	setType(RtmpMessageType::VideoMessage);
}

const v8& RtmpVideoMessage::serialize()
{
	if (!payload_.empty()) return payload_;
	RtmpMediaMessage::serialize();

	// RTMP Body
	auto specs = videoTag_.serialize();
	std::move(specs.begin(), specs.end(), std::back_inserter(payload_));

  	return payload_;
}

void RtmpVideoMessage::deserialize()
{
	if (videoTag_.payload().empty())
	{
		videoTag_.setPayloadSize(payloadSize());
		
		auto data = v8{};
		std::move(payload_.begin(), payload_.end(), std::back_inserter(data));
		videoTag_.appendPayload(std::move(data));
		
		videoTag_.deserialize();
	}
	
	payload_.clear();
}

void RtmpVideoMessage::appendPayload(v8&& data)
{
	videoTag_.appendPayload(std::forward<v8>(data));
}

void RtmpVideoMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	RtmpMediaMessage::deserialize(begin, end);
}

const v8& RtmpVideoMessage::data() const
{
	return videoTag_.payload();
}

size_t RtmpVideoMessage::size() const
{
	return payloadSize();
}

FLVVideoCodecType RtmpVideoMessage::codecId() const
{
	return videoTag_.codecId();
}

void RtmpVideoMessage::setCodecId(FLVVideoCodecType value)
{
	videoTag_.setCodecId(value);
}

FLVVideoFrameType RtmpVideoMessage::frameType() const
{
	return videoTag_.frameType();
}

void RtmpVideoMessage::setFrameType(FLVVideoFrameType value)
{
	videoTag_.setFrameType(value);
}
