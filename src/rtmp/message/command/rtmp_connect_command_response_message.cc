#include "../../../../private/rtmp/message/command/rtmp_connect_command_response_message.h"
#include "../../../../private/amf/amf.h"
using namespace RtmpKit;

void RtmpConnectCommandResponseMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
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

void RtmpConnectCommandResponseMessage::deserialize()
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

		// Optional User object properties
		const auto *userPropsVariant = boost::get<AMF::ObjectVariant>(&item);
		if (userPropsVariant != nullptr && optionalUserArguments_.empty())
		{
			optionalUserArguments_ = userPropsVariant->value();
			continue;
		}
	}
}

std::size_t RtmpConnectCommandResponseMessage::size() const
{
	return payloadSize();
}

std::size_t RtmpConnectCommandResponseMessage::headerSize()
{
	return 12;
}

bool RtmpConnectCommandResponseMessage::isSuccessfull() const
{
	if (optionalUserArguments_.empty())
	{
		return false;
	}

	const auto code = optionalUserArguments_.find("code");

	if (code != optionalUserArguments_.cend())
	{
		const auto *resultVariant = boost::get<AMF::StringVariant>(&code->second);
		return (resultVariant != nullptr && resultVariant->value() != "error");
	}

	return false;
}

std::string RtmpConnectCommandResponseMessage::description() const
{
	if (optionalUserArguments_.empty())
	{
		return "";
	}

	const auto desc = optionalUserArguments_.find("description");

	if (desc != optionalUserArguments_.cend())
	{
		const auto *resultVariant = boost::get<AMF::StringVariant>(&desc->second);

		if (resultVariant != nullptr)
		{
			return resultVariant->value();
		}
	}

	return "";
}
