#include "../../../../private/rtmp/message/command/rtmp_fcpublish_command_message.h"
#include "../../../../private/amf/amf.h"
using namespace RtmpKit;

RtmpFCPublishCommandMessage::RtmpFCPublishCommandMessage()
	: publishingName_("")
{
	setTransactionID(2);
	setCommandName("FCPublish");
	setType(RtmpMessageType::CommandMessage_AMF0);
}

RtmpFCPublishCommandMessage::RtmpFCPublishCommandMessage(const std::string& publishingName)
	: publishingName_(publishingName)
{
	setTransactionID(2);
	setCommandName("FCPublish");
	setType(RtmpMessageType::CommandMessage_AMF0);
}

const v8& RtmpFCPublishCommandMessage::serialize()
{
	payload_.clear();
	auto serializer = AMF::v0::Serializer{};

	// RTMP Header
	payload_.push_back(RtmpMessage::chunkStreamID(chunkStreamType()));

	auto time = RtmpKit::toBigEndian24(timestamp());
	std::move(time.begin(), time.end(), back_inserter(payload_));

	payload_.push_back(static_cast<u8>(type()));

	auto sID = RtmpKit::toLittleEndian32(streamID());
	std::move(sID.begin(), sID.end(), back_inserter(payload_));

	// RTMP Body
	auto cName = serializer.serializeString(commandName());
	std::move(cName.begin(), cName.end(), std::back_inserter(payload_));

	auto transID = serializer.serializeNumber(static_cast<double>(transactionID()));
	std::move(transID.begin(), transID.end(), std::back_inserter(payload_));

	auto size = static_cast<u24>(cName.size() + transID.size());

	auto commandObjectVariant = serializer.serializeNullObject();
	std::move(commandObjectVariant.begin(), commandObjectVariant.end(), std::back_inserter(payload_));
	size += static_cast<u24>(commandObjectVariant.size());

	auto publishingNameVariant = serializer.serializeString(publishingName());
	std::move(publishingNameVariant.begin(), publishingNameVariant.end(), std::back_inserter(payload_));
	size += static_cast<u24>(publishingNameVariant.size());

	setPayloadSize(size);
	auto pSize = RtmpKit::toBigEndian24(payloadSize());
	payload_.insert(payload_.begin() + 4, pSize.begin(), pSize.end());

	return payload_;
}
