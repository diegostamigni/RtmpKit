#include "../../../private/rtmp/message/rtmp_audio_message.h"
using namespace RtmpKit;

RtmpAudioMessage::RtmpAudioMessage(u8 chunkStreamId, v8&& data)
{
	setChunkStreamFmt(chunkStreamId);
	setChunkStreamType(RtmpChunkStreamType::Type3);
	setType(RtmpMessageType::AudioMessage);
	setPayloadSize(static_cast<u24>(data.size()));

	audioTag_.setPayload(std::forward<v8>(data));
	audioTag_.deserialize();
}

RtmpAudioMessage::RtmpAudioMessage(FLVMediaAudioTag&& tag)
	: audioTag_(std::forward<FLVMediaAudioTag>(tag))
{
	setChunkStreamFmt(static_cast<u8>(RtmpChunkStreamFmt::AudioMessage));
	setTimestamp(tag.timestamp());
	setPayloadSize(tag.payloadSize());
	setType(RtmpMessageType::AudioMessage);
}

const v8& RtmpAudioMessage::serialize()
{
	if (!payload_.empty()) return payload_;
	RtmpMediaMessage::serialize();

	// RTMP Body
	auto specs = audioTag_.serialize();
	std::move(specs.begin(), specs.end(), std::back_inserter(payload_));

	return payload_;
}

void RtmpAudioMessage::deserialize()
{
	if (audioTag_.payload().empty())
	{
		audioTag_.setPayloadSize(payloadSize());
		
		auto data = v8{};
		std::move(payload_.begin(), payload_.end(), std::back_inserter(data));
		audioTag_.appendPayload(std::move(data));
		
		audioTag_.deserialize();
	}
	
	payload_.clear();
}

void RtmpAudioMessage::appendPayload(v8&& data)
{
	audioTag_.appendPayload(std::forward<v8>(data));
}

void RtmpAudioMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	RtmpMediaMessage::deserialize(begin, end);
}

u32 RtmpAudioMessage::delay() const
{
	return 0;
}

const v8& RtmpAudioMessage::data() const
{
	return audioTag_.payload();
}

size_t RtmpAudioMessage::size() const
{
	return payloadSize();
}

FLVSoundFormatType RtmpAudioMessage::soundFormat() const
{
	return audioTag_.soundFormat();
}

void RtmpAudioMessage::setSoundFormat(FLVSoundFormatType value)
{
	audioTag_.setSoundFormat(value);
}

FLVSoundRateType RtmpAudioMessage::soundRate() const
{
	return audioTag_.soundRate();
}

void RtmpAudioMessage::setSoundRate(FLVSoundRateType value)
{
	audioTag_.setSoundRate(value);
}

FLVSoundSampleSizeType RtmpAudioMessage::soundSampleSize() const
{
	return audioTag_.soundSampleSize();
}

void RtmpAudioMessage::setSoundSampleSize(FLVSoundSampleSizeType value)
{
	audioTag_.setSoundSampleSize(value);
}

FLVSoundType RtmpAudioMessage::soundType() const
{
	return audioTag_.soundType();
}

void RtmpAudioMessage::setSoundType(FLVSoundType value)
{
	audioTag_.setSoundType(value);
}

FLVAACPacketType RtmpAudioMessage::aacPacketType() const
{
	return audioTag_.aacPacketType();
}

void RtmpAudioMessage::setAACPacketType(FLVAACPacketType value)
{
	audioTag_.setAACPacketType(value);
}
