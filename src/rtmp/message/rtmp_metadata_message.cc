#include "../../../private/rtmp/message/rtmp_metadata_message.h"
#include "../../../private/amf/amf.h"
using namespace RtmpKit;

RtmpMetadataMessage::RtmpMetadataMessage()
{
	setType(RtmpMessageType::DataMessage_AMF0);
}

RtmpMetadataMessage::RtmpMetadataMessage(const FLVMediaAMFTag& metadata)
	: metadata_(metadata)
{
	setType(RtmpMessageType::DataMessage_AMF0);
}

const v8& RtmpMetadataMessage::serialize()
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
	std::size_t size = 0;
	auto serializer = AMF::v0::Serializer{};
	
	auto setDataFrame = serializer.serializeString("@setDataFrame");
	size += setDataFrame.size();
	std::move(setDataFrame.begin(), setDataFrame.end(), std::back_inserter(payload_));
	
	auto medataSerialized = metadata_.serialize();
	size += medataSerialized.size();
	std::move(medataSerialized.begin(), medataSerialized.end(), std::back_inserter(payload_));

	setPayloadSize(static_cast<u24>(size));
	auto pSize = RtmpKit::toBigEndian24(payloadSize());
	payload_.insert(payload_.begin() + 4, pSize.begin(), pSize.end());

	return payload_;
}

std::size_t RtmpMetadataMessage::size() const
{
	return payloadSize();
}

void RtmpMetadataMessage::deserialize()
{
	metadata_.setPayloadSize(payloadSize());
	metadata_.setPayload({ payload_.cbegin(), payload_.cend() });
	metadata_.deserialize();
	payload_.erase(payload_.cbegin());
}

void RtmpMetadataMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	RtmpMediaMessage::deserialize(begin, end);
}

double RtmpMetadataMessage::width() const
{
	return metadata_.width();
}

void RtmpMetadataMessage::setWidth(double width)
{
	metadata_.setWidth(width);
}

double RtmpMetadataMessage::height() const
{
	return metadata_.height();
}

void RtmpMetadataMessage::setHeight(double height)
{
	metadata_.setHeight(height);
}

double RtmpMetadataMessage::videoDataRate() const
{
	return metadata_.videoDataRate();
}

void RtmpMetadataMessage::setVideoDataRate(double videodatarate)
{
	metadata_.setVideoDataRate(videoDataRate());
}

double RtmpMetadataMessage::videoFrameRate() const
{
	return metadata_.videoFrameRate();
}

void RtmpMetadataMessage::setVideoFrameRate(double value)
{
	metadata_.setVideoFrameRate(value);
}

double RtmpMetadataMessage::frameRate() const
{
	return metadata_.frameRate();
}

void RtmpMetadataMessage::setFrameRate(double framerate)
{
	metadata_.setFrameRate(frameRate());
}

const std::string& RtmpMetadataMessage::videoCodecId() const
{
	return metadata_.videoCodecIdStr();
}

void RtmpMetadataMessage::setVideoCodecId(const std::string& value)
{
	metadata_.setVideoCodecIdStr(value);
}

double RtmpMetadataMessage::audioDataRate() const
{
	return metadata_.audioDataRate();
}

void RtmpMetadataMessage::setAudioDataRate(double audiodatarate)
{
	metadata_.setAudioDataRate(audioDataRate());
}

double RtmpMetadataMessage::audioSampleRate() const
{
	return metadata_.audioSampleRate();
}

void RtmpMetadataMessage::setAudioSampleRate(double audiosamplerate)
{
	metadata_.setAudioSampleRate(audioSampleRate());
}

double RtmpMetadataMessage::audioChannels() const
{
	return metadata_.audioChannels();
}

void RtmpMetadataMessage::setAudioChannels(double value)
{
	metadata_.setAudioChannels(value);
}

const std::string& RtmpMetadataMessage::audioCodecId() const
{
	return metadata_.audioCodecIdStr();
}

void RtmpMetadataMessage::setAudioCodecId(const std::string& value)
{
	metadata_.setAudioCodecIdStr(value);
}
