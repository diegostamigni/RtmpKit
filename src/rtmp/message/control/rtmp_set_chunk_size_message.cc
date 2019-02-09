#include "../../../../private/rtmp/message/control/rtmp_set_chunk_size_message.h"
using namespace RtmpKit;

RtmpSetChunkSizeMessage::RtmpSetChunkSizeMessage(u32 size)
	: chunkSize_(size)
{
	setType(RtmpMessageType::SetChunkSizeMessage);
	setStreamID(0);
	setTimestamp(0);
	setPayloadSize(sizeof(decltype(chunkSize_)));
}

const v8& RtmpSetChunkSizeMessage::serialize()
{
	payload_.clear();

	// RTMP Header
	payload_.push_back(RtmpMessage::chunkStreamID(chunkStreamType_));

	auto time = RtmpKit::toBigEndian24(timestamp());
	std::move(time.begin(), time.end(), back_inserter(payload_));

	auto pSize = RtmpKit::toBigEndian24(payloadSize());
	std::move(pSize.begin(), pSize.end(), back_inserter(payload_));

	payload_.push_back(static_cast<u8>(type()));

	auto sID = RtmpKit::toLittleEndian32(streamID());
	std::move(sID.begin(), sID.end(), back_inserter(payload_));

	// RTMP Body
	auto cSize = toBigEndian32(chunkSize());
	std::move(cSize.begin(), cSize.end(), std::back_inserter(payload_));

	return payload_;
}

void RtmpSetChunkSizeMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	auto iter = begin;

	// const auto chunkStreamID = (static_cast<RtmpChunkStreamID>(*iter));
	iter += 1;

	setTimestamp(fromBigEndian24({iter, iter + 3}));
	iter += 3;

	setPayloadSize(fromBigEndian24({iter, iter + 3}));
	iter += 3;

	setType(static_cast<RtmpMessageType>(*iter));
	iter += 1;

	setStreamID(fromLittleEndian32({iter, iter + 4}));
	iter += 4;

	setPayload({iter, iter + payloadSize()});
	setChunkSize(fromBigEndian32(payload_));
}

std::size_t RtmpSetChunkSizeMessage::size() const
{
	return RtmpSetChunkSizeMessage::defaultSize();
}

std::size_t RtmpSetChunkSizeMessage::defaultSize()
{
	return sizeof(u32) + 12 /* header size */;
}
