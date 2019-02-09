#include "../../../../private/rtmp/message/command/rtmp_connect_command_message.h"
#include "../../../../private/amf/amf.h"
#include "../../../../private/logger.h"

using namespace RtmpKit;

RtmpConnectCommandMessage::RtmpConnectCommandMessage(const std::string& address,
	const std::string& port, const std::string& appName)
{
	setType(RtmpMessageType::CommandMessage_AMF0);
	setCommandName("connect");
	setTransactionID(1);
	setStreamID(0);
	setTimestamp(0);

	addCommandObjectProperty(PROPERTY_FPAD, AMF::BooleanVariant(false));
	addCommandObjectProperty(PROPERTY_OBJECT_ENCODING,
		AMF::NumberVariant(static_cast<u32>(RtmpObjectEncodingType::AMF0)));

	addCommandObjectProperty(PROPERTY_APP, AMF::StringVariant(appName));

	// TODO(diegostamigni): check if the final URL is valid (and if already contains "rtmp://")
	const auto url = str(boost::format("rtmp://%1%:%2%/%3%") % address % port % appName);
	LOG_INFO(str(boost::format("Using %1%: %2% (appName: %3%)") % PROPERTY_TC_URL % url % appName).c_str());
	addCommandObjectProperty(PROPERTY_TC_URL, AMF::StringVariant(url));

	// TODO(diegostamigni): NumberVariant seems to not works properly in this case
//	addCommandObjectProperty(PROPERTY_AUDIO_CODECS,
//		AMF::NumberVariant(static_cast<u32>(RtmpAudioCodecType::SUPPORT_SND_MP3)));
//	addCommandObjectProperty(PROPERTY_VIDEO_CODECS,
//		AMF::NumberVariant(static_cast<u32>(RtmpVideoCodecType::SUPPORT_VID_H264)));
}

const v8& RtmpConnectCommandMessage::serialize()
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

	if (!commandObjectProperties().empty())
	{
		auto coProperties = serializer.serializeObject(commandObjectProperties());
		std::move(coProperties.begin(), coProperties.end(), std::back_inserter(payload_));
		size += static_cast<u24>(coProperties.size());
	}

	if (!optionalUserArgumentsProperties().empty())
	{
		auto userProperties = serializer.serializeObject(optionalUserArgumentsProperties());
		std::move(userProperties.begin(), userProperties.end(), std::back_inserter(payload_));
		size += static_cast<u24>(userProperties.size());
	}

	setPayloadSize(size);
	auto pSize = RtmpKit::toBigEndian24(payloadSize());
	payload_.insert(payload_.begin() + 4, pSize.begin(), pSize.end());

	return payload_;
}
