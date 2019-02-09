#include "../../../../private/rtmp/message/control/rtmp_window_ack_size_message.h"
using namespace RtmpKit;

RtmpWindowAcknowledgementSizeMessage::RtmpWindowAcknowledgementSizeMessage()
{
	setChunkStreamFmt(static_cast<u8>(RtmpChunkStreamFmt::ControlMessage));
}

RtmpWindowAcknowledgementSizeMessage::RtmpWindowAcknowledgementSizeMessage(u32 windowAcknowledgementSize)
	: windowAcknowledgementSize_(windowAcknowledgementSize)
{
	setChunkStreamFmt(static_cast<u8>(RtmpChunkStreamFmt::ControlMessage));
	setType(RtmpMessageType::WindowAcknowledgementSizeMessage);
}

const v8& RtmpWindowAcknowledgementSizeMessage::serialize()
{
	payload_.clear();

	// RTMP Header
	payload_.push_back(RtmpMessage::chunkStreamID(chunkStreamType()));

	auto time = RtmpKit::toBigEndian24(timestamp());
	std::move(time.begin(), time.end(), back_inserter(payload_));

	payload_.push_back(static_cast<u8>(type()));

	auto sID = RtmpKit::toLittleEndian32(streamID());
	std::move(sID.begin(), sID.end(), back_inserter(payload_));

	// RTMP Body
	auto vWAS = toBigEndian32(windowAcknowledgementSize());
	std::move(vWAS.begin(), vWAS.end(), std::back_inserter(payload_));

	setPayloadSize(static_cast<u24>(sizeof(decltype(windowAcknowledgementSize_))));
	auto pSize = RtmpKit::toBigEndian24(payloadSize());
	payload_.insert(payload_.begin() + 4, pSize.begin(), pSize.end());

	return payload_;
}

void RtmpWindowAcknowledgementSizeMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
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
	setWindowAcknowledgementSize(fromBigEndian32(payload_));
}

std::size_t RtmpWindowAcknowledgementSizeMessage::size() const
{
	return RtmpWindowAcknowledgementSizeMessage::defaultSize();
}

std::size_t RtmpWindowAcknowledgementSizeMessage::defaultSize()
{
	return sizeof(u32) + 12 /* header size */;
}
