#include "../../../private/rtmp/message/rtmp_media_message.h"
#include "../../../private/logger.h"

using namespace RtmpKit;

const v8& RtmpMediaMessage::serialize()
{
	if (!payload_.empty())
	{
		return payload_;
	}
	
	// RTMP Header
	payload_.push_back(RtmpMessage::chunkStreamID(chunkStreamType()));
	
	switch (chunkStreamType())
	{
		case RtmpChunkStreamType::Type0:
		case RtmpChunkStreamType::Type1:
		case RtmpChunkStreamType::Type2:
		{
			auto time = RtmpKit::toBigEndian24(timestamp());
			std::move(time.begin(), time.end(), std::back_inserter(payload_));
			
			if (chunkStreamType() == RtmpChunkStreamType::Type2)
			{
				break;
			}
			
			auto pSize = RtmpKit::toBigEndian24(payloadSize());
			std::move(pSize.begin(), pSize.end(), std::back_inserter(payload_));
			
			payload_.push_back(static_cast<u8>(type()));
			
			if (chunkStreamType() == RtmpChunkStreamType::Type0)
			{
				auto sID = RtmpKit::toLittleEndian32(streamID());
				std::move(sID.begin(), sID.end(), std::back_inserter(payload_));
			}
			
			break;
		}
			
		case RtmpChunkStreamType::Type3:
		{
			break;
		}
			
		default:
			break;
	}
	
	return payload_;
}

void RtmpMediaMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	if (!payload_.empty())
	{
		return;
	}
	
	auto iter = begin;
	
	setChunkStreamType(static_cast<RtmpChunkStreamType>((*iter & 0xff) >> 6));
	iter += 1;
	
	switch (chunkStreamType())
	{
		case RtmpChunkStreamType::Type0:
		case RtmpChunkStreamType::Type1:
		case RtmpChunkStreamType::Type2:
		{
			setTimestamp(fromBigEndian24({ iter, iter + 3 }));
			iter += 3;
			
			if (chunkStreamType() == RtmpChunkStreamType::Type2)
			{
				break;
			}
			
			setPayloadSize(fromBigEndian24({ iter, iter + 3 }));
			iter += 3;
			
			setType(static_cast<RtmpMessageType>(*iter));
			iter += 1;
			
			if (hasExtendedTimestamp())
			{
				setExtendedTimestamp(fromLittleEndian32({ iter, iter + 4 }));
				iter += 4;
			}
			
			if (chunkStreamType() == RtmpChunkStreamType::Type0)
			{
				setStreamID(fromLittleEndian32({ iter, iter + 4 }));
				iter += 4;
			}
			
			break;
		}
			
		case RtmpChunkStreamType::Type3:
		{
			break;
		}
			
		default:
			break;
	}
}

auto RtmpMediaMessage::trunkToSize(u32 maxSize) -> std::vector<v8>
{
	if (payload_.empty()) serialize();
	if (payload_.empty()) return {};
	
	const auto currentSize = payload_.size() - hSize();
	if (currentSize <= maxSize) return {};
	
	/*LOG_DEBUG
		<< "trunkToSize(...) required, packet size is " << currentSize
		<< " but max allowed size is " << maxSize
	;*/
	
	auto result = std::vector<v8>{};
	auto iter = payload_.begin() + hSize() + maxSize;

	const auto defaultCopySize = static_cast<int32_t>(maxSize);
	const auto distanceSize = std::distance(iter, payload_.end());
	auto chunkSize = distanceSize >= defaultCopySize ? defaultCopySize : distanceSize;

	while (chunkSize > 0)
	{
		if (iter <= payload_.end())
		{
			auto data = v8{};
			std::move(iter, iter + chunkSize, std::back_inserter(data));
			
			iter += chunkSize;
			result.push_back(std::move(data));
			
			const auto remainingSize = std::distance(iter, payload_.end());
			chunkSize = remainingSize >= defaultCopySize ? defaultCopySize : remainingSize;
		}
		else
		{
			break;
		}
	}
	
	payload_.erase(payload_.begin() + hSize() + maxSize, payload_.end());
	return result;
}

void RtmpMediaMessage::updatePayloadSize(u32 size)
{
	if (payload_.empty() || payload_.size() < hSize())
	{
		return;
	}

	setPayloadSize(size);
	const auto pSize = RtmpKit::toBigEndian24(size);
	const auto cbegin = payload_.cbegin() + 4;
	const auto cend = cbegin + pSize.size();

	payload_.erase(cbegin, cend);
	payload_.insert(payload_.cbegin() + 4, pSize.cbegin(), pSize.cend());
}
	
void RtmpMediaMessage::updateChunkStreamId(u8 fmt)
{
	if (payload_.empty())
	{
		return;
	}
	
	auto iter = payload_.begin();
	setChunkStreamFmt(fmt);
	*iter = chunkStreamID(chunkStreamType());
}
