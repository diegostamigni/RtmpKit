#include "../../../../private/rtmp/message/command/rtmp_close_stream_command_message.h"
#include "../../../../private/amf/amf.h"
using namespace RtmpKit;

RtmpCloseStreamCommandMessage::RtmpCloseStreamCommandMessage()
{
	setType(RtmpMessageType::CommandMessage_AMF0);
	setChunkStreamType(RtmpChunkStreamType::Type0);
	setCommandName("closeStream");
	setTransactionID(1);
	setStreamID(0);
	setTimestamp(0);
}

const v8& RtmpCloseStreamCommandMessage::serialize()
{
	payload_.clear();
	auto serializer = AMF::v0::Serializer{};

	// RTMP Header
	payload_.push_back(RtmpMessage::chunkStreamID(chunkStreamType()));

	auto time = RtmpKit::toBigEndian24(timestamp());
	std::move(time.begin(), time.end(), back_inserter(payload_));

	payload_.push_back(static_cast<u8>(type()));

	switch (chunkStreamType())
	{
	case RtmpChunkStreamType::Type0:
	{
		auto sID = RtmpKit::toLittleEndian32(streamID());
		std::move(sID.begin(), sID.end(), std::back_inserter(payload_));
		break;
	}
	default:
		break;
	}

	// RTMP Body
	auto cName = serializer.serializeString(commandName());
	std::move(cName.begin(), cName.end(), std::back_inserter(payload_));

	auto transID = serializer.serializeNumber(static_cast<double>(transactionID()));
	std::move(transID.begin(), transID.end(), std::back_inserter(payload_));

	auto size = static_cast<u24>(cName.size() + transID.size());

	if (!commandObjectProperties().empty())
	{
		auto coProperties = serializer.serializeObject(commandObjectProperties());
		std::move(coProperties.begin(), coProperties.end(), std::back_inserter(payload_));
		size += static_cast<u24>(coProperties.size());
	}
	else
	{
		auto nullProperties = serializer.serializeNullObject();
		std::move(nullProperties.begin(), nullProperties.end(), std::back_inserter(payload_));
		size += static_cast<u24>(nullProperties.size());
	}

	setPayloadSize(size);
	auto pSize = RtmpKit::toBigEndian24(payloadSize());
	payload_.insert(payload_.begin() + 4, pSize.begin(), pSize.end());

	return payload_;
}
