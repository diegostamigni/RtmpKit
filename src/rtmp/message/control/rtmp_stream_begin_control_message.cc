#include "../../../../private/rtmp/message/control/rtmp_stream_begin_control_message.h"

using namespace RtmpKit;

RtmpStreamBeginControlMessage::RtmpStreamBeginControlMessage()
{
	setChunkStreamFmt(static_cast<u8>(RtmpChunkStreamFmt::ControlMessage));
}

void RtmpStreamBeginControlMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	auto iter = begin;

	// RTMP Header
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
}

void RtmpStreamBeginControlMessage::deserialize()
{
	// TODO(diegostamigni): implement
}

std::size_t RtmpStreamBeginControlMessage::size() const
{
	return payloadSize();
}

std::size_t RtmpStreamBeginControlMessage::headerSize()
{
	return 12;
}

bool RtmpStreamBeginControlMessage::isSuccessfull() const
{
	return true;
}

std::string RtmpStreamBeginControlMessage::description() const
{
	return "";
}
