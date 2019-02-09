#include "../../../private/rtmp/message/rtmp_message.h"
using namespace RtmpKit;

void RtmpMessage::deserialize()
{
}

void RtmpMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	auto iter = begin;
	
	// RTMP Header
	setChunkStreamType(static_cast<RtmpChunkStreamType>((*iter & 0xff) >> 6));
	iter += 1;
	
	setTimestamp(fromBigEndian24({ iter, iter + 3 }));
	iter += 3;
	
	setPayloadSize(fromBigEndian24({ iter, iter + 3 }));
	iter += 3;
	
	setType(static_cast<RtmpMessageType>(*iter));
	iter += 1;
	
	if (hasExtendedTimestamp())
	{
		setExtendedTimestamp(fromLittleEndian32({ iter, iter + 4 }));
		iter += 4;
	}
	
	switch (chunkStreamType())
	{
		case RtmpChunkStreamType::Type0:
		{
			setStreamID(fromLittleEndian32({ iter, iter + 4 }));
			iter += 4;
			break;
		}
			
		default:
			break;
	}
	
	setPayload({iter, iter + payloadSize()});
}

const v8& RtmpMessage::serialize()
{
	return payload_;
}

std::size_t RtmpMessage::size() const
{
	return payload_.size();
}

u8 RtmpMessage::chunkStreamID(RtmpChunkStreamType streamFormatType) const
{
	const auto value = static_cast<u8>(streamFormatType) << 6 | chunkStreamId_;
	return value;
}

std::vector<v8> RtmpMessage::trunkToSize(u32 size)
{
	return { payload_ };
}

u32 RtmpMessage::delay() const
{
	const auto &payload = data();
	if (payload.size() < 5) return 0;
	auto iter = payload.cbegin() + 2;
	auto delay = RtmpKit::fromBigEndian24({iter, iter + 3});
	return delay;
}

std::size_t RtmpMessage::hSize() const
{
	switch (chunkStreamType())
	{
		case RtmpChunkStreamType::Type0:
			return hasExtendedTimestamp() ? 12 + sizeof(u32) : 12;
			
		case RtmpChunkStreamType::Type1:
			return hasExtendedTimestamp() ? 8 + sizeof(u32) : 8;
			
		case RtmpChunkStreamType::Type2:
			return hasExtendedTimestamp() ? 4 + sizeof(u32) : 4;
			
		case RtmpChunkStreamType::Type3:
			return hasExtendedTimestamp() ? 1 + sizeof(u32) : 1;
			
		default:
			return 1;
	}
}
