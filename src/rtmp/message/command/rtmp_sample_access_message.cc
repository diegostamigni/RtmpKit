#include "../../../../private/rtmp/message/command/rtmp_sample_access_message.h"
#include "../../../../private/amf/amf.h"
using namespace RtmpKit;

void RtmpSampleAccessCommandMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	auto iter = begin;

	// RTMP Header
	// const auto chunkStreamID = (static_cast<RtmpChunkStreamID>(*iter));
	iter += 1;

	setTimestamp(fromBigEndian24({ iter, iter + 3 }));
	iter += 3;

	setPayloadSize(fromBigEndian24({ iter, iter + 3 }));
	iter += 3;

	setType(static_cast<RtmpMessageType>(*iter));
	iter += 1;

	setStreamID(fromLittleEndian32({ iter, iter + 4 }));
	iter += 4;
}

void RtmpSampleAccessCommandMessage::deserialize()
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

		const auto *isAudioAllowedVariant = boost::get<AMF::BooleanVariant>(&item);
		if (isAudioAllowedVariant != nullptr && !isAudioAllowedSet_)
		{
			isAudioAllowedSet_ = true;
			isAudioAllowed_ = isAudioAllowedVariant->value();
			continue;
		}

		const auto *isVideoAllowedVariant = boost::get<AMF::BooleanVariant>(&item);
		if (isVideoAllowedVariant != nullptr && !isVideoAllowedSet_)
		{
			isVideoAllowedSet_ = true;
			isAudioAllowed_ = isVideoAllowedVariant->value();
			continue;
		}
	}
}

size_t RtmpSampleAccessCommandMessage::size() const
{
	return payloadSize();
}

std::size_t RtmpSampleAccessCommandMessage::headerSize()
{
	return 12;
}

bool RtmpSampleAccessCommandMessage::isSuccessfull() const
{
	return isVideoAllowed() || isAudioAllowed();
}

std::string RtmpSampleAccessCommandMessage::description() const
{
	return "";
}

