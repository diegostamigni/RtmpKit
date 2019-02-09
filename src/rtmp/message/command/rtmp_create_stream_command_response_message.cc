#include "../../../../private/rtmp/message/command/rtmp_create_stream_command_response_message.h"
#include "../../../../private/amf/amf.h"
using namespace RtmpKit;

void RtmpCreateStreamCommandResponseMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
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

void RtmpCreateStreamCommandResponseMessage::deserialize()
{
	const auto deserializer = AMF::v0::Deserializer{};
	const auto values = deserializer.deserialize(payload());

	for (const auto &item : values)
	{
		const auto *resultVariant = boost::get<AMF::StringVariant>(&item);
		if (resultVariant != nullptr)
		{
			setCommandName(resultVariant->value());
			continue;
		}

		const auto *trIDVariant = boost::get<AMF::NumberVariant>(&item);
		if (trIDVariant != nullptr)
		{
			setTransactionID(static_cast<u8>(trIDVariant->value()));
			continue;
		}

		// Command object properties
		const auto *objPropsVariant = boost::get<AMF::ObjectVariant>(&item);
		if (objPropsVariant != nullptr && commandObject_.empty())
		{
			commandObject_ = objPropsVariant->value();
			continue;
		}
	}
}

std::size_t RtmpCreateStreamCommandResponseMessage::size() const
{
	return payloadSize();
}

std::size_t RtmpCreateStreamCommandResponseMessage::headerSize()
{
	return 12;
}

bool RtmpCreateStreamCommandResponseMessage::isSuccessfull() const
{
	// TODO (diegostamigni): verify if the get some sort of NetStream.CreateStream.Successfull like with the `connect' command
	return true;
}

std::string RtmpCreateStreamCommandResponseMessage::description() const
{
	// TODO (diegostamigni): verify if the get some sort of `description' field like with the `connect' command
	return "";
}
