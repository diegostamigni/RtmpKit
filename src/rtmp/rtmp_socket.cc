#include "../../private/rtmp/rtmp_socket.h"
#include "../../private/logger.h"

using namespace RtmpKit;

void RtmpSocket::receiveDynamicPacketWithDynamicHeader(u32 maxSize,
    const std::function<void(const boost::system::error_code& ec,
        std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback)
{
    RtmpChunk chunks;
    
    receiveChunk(chunks, maxSize, [maxSize, &chunks, callback, this](const auto& ec, auto size, auto&& packet) mutable
	{
		auto error = ec;
		if (!error)
		{
			auto &totalSize = std::get<1>(chunks); // total size
			auto &chunkSize = std::get<2>(chunks); // already retrieved
			auto nextChunkSize = (totalSize - chunkSize);
			chunkSize = nextChunkSize > maxSize ? maxSize : nextChunkSize;
			
			while (chunkSize > 0)
			{
				totalSize = nextChunkSize;
				
				this->receiveChunk(chunks, maxSize, [&error, &chunkSize](const auto& ec, auto size, auto&& packet)
				{
					if (ec) // error
					{
						chunkSize = 0; // will cause the while loop to end
						error = ec; // we need to save the error
					}
				});
				
				auto &tS = std::get<1>(chunks);
				auto &cS = std::get<2>(chunks);
				
				nextChunkSize = (tS - cS);
				chunkSize = nextChunkSize > maxSize ? maxSize : nextChunkSize;
			}
			
			auto data = std::move(std::get<v8>(chunks));
			
			if (!data.empty())
			{
				if (auto *p = boost::get<RtmpAudioMessage>(&packet))
				{
					p->appendPayload(std::move(data));
					p->deserialize();
				}
				
				if (auto *p = boost::get<RtmpVideoMessage>(&packet))
				{
					p->appendPayload(std::move(data));
					p->deserialize();
				}
				
				if (auto *p = boost::get<RtmpMetadataMessage>(&packet))
				{
					p->appendPayload(std::move(data));
					p->deserialize();
				}
			}
		}
		
		callback(error, size, std::move(packet));
    });
}

void RtmpSocket::receiveChunk(RtmpChunk& chunks, u32 maxSize,
    const std::function<void(const boost::system::error_code& ec,
        std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback)
{
    receiveData(sizeof(RtmpChunkStreamType),
        [maxSize, &chunks, callback, this](const auto& ec, auto size, auto data) mutable
    {
        if (!ec && data->size() == sizeof(RtmpChunkStreamType))
        {
            const auto rtmpFormat = ((*(data->cbegin())) & 0xff) >> 6;
            
            if (rtmpFormat < 0 || rtmpFormat > 3)
            {
				LOG_DEBUG(str(boost::format("Unsupported RTMP format received: %1%") % rtmpFormat).c_str());
                callback(boost::asio::error::invalid_argument, size, RtmpMediaPacketVariant());
                return;
            }
            
            auto header = v8{};
            const auto fmt = static_cast<RtmpChunkStreamType>(rtmpFormat);
            std::move(data->begin(), data->end(), std::back_inserter(header));
            
            switch (fmt)
            {
                case RtmpChunkStreamType::Type0:
                case RtmpChunkStreamType::Type1:
                case RtmpChunkStreamType::Type2:
                {
                    this->receiveTimestamp(fmt, maxSize, std::move(header), chunks, callback);
                    break;
                }
                    
                case RtmpChunkStreamType::Type3:
                {
                    this->receivePayload(fmt, maxSize, std::move(header), chunks, callback);
                    break;
                }
            }
        }
        else
        {
            callback(ec, size, RtmpMediaPacketVariant());
        }
    });
}

void RtmpSocket::receiveTimestamp(RtmpChunkStreamType chunkStreamType, u32 maxSize, v8&& readData, RtmpChunk& chunks,
    const std::function<void(const boost::system::error_code& ec,
        std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback)
{
    receiveData(sizeof(u32) - 1, [header = std::forward<v8>(readData), chunkStreamType, maxSize, &chunks, callback, this]
		(const auto& ec, auto size, auto data) mutable
    {
        if (!ec)
		{
			const auto timestamp = RtmpKit::fromBigEndian24(*data);
			std::move(data->begin(), data->end(), std::back_inserter(header));
			
			switch (chunkStreamType)
			{
				case RtmpChunkStreamType::Type0:
				case RtmpChunkStreamType::Type1:
				{
					this->receiveBodySize(chunkStreamType, maxSize, RtmpKit::numberOfSetBits(timestamp) >= 24,
						std::move(header), chunks, callback);
					break;
				}
					
				case RtmpChunkStreamType::Type2:
				case RtmpChunkStreamType::Type3:
				{
					this->receivePayload(chunkStreamType, maxSize, std::move(header), chunks, callback);
					break;
				}
			}
        }
        else
        {
            callback(ec, size, RtmpMediaPacketVariant());
        }
    });
}

void RtmpSocket::receiveBodySize(RtmpChunkStreamType chunkStreamType,u32 maxSize,  bool hasExtTimestamp, v8&& readData,
    RtmpChunk& chunks, const std::function<void(const boost::system::error_code& ec,
        std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback)
{
    receiveData(sizeof(u32) - 1,
        [header = std::forward<v8>(readData), chunkStreamType, maxSize, hasExtTimestamp, &chunks, callback, this]
            (const auto& ec, auto size, auto data) mutable
    {
        if (!ec)
        {
            std::move(data->begin(), data->end(), std::back_inserter(header));
            this->receiveMessageType(chunkStreamType, maxSize, hasExtTimestamp, std::move(header), chunks, callback);
        }
        else
        {
            callback(ec, size, RtmpMediaPacketVariant());
        }
    });
}

void RtmpSocket::receiveMessageType(RtmpChunkStreamType chunkStreamType, u32 maxSize, bool hasExtTimestamp,
    v8&& readData, RtmpChunk& chunks, const std::function<void(const boost::system::error_code& ec,
        std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback)
{
    receiveData(sizeof(RtmpMessageType), [header = std::forward<v8>(readData), chunkStreamType, maxSize,
		hasExtTimestamp, &chunks, callback, this](const auto& ec, auto size, auto data) mutable
    {
        if (!ec)
        {
            std::move(data->begin(), data->end(), std::back_inserter(header));
			
			if (hasExtTimestamp)
			{
				this->receiveExtentedTimestamp(chunkStreamType, maxSize, std::move(header), chunks, callback);
			}
			else
			{
				switch (chunkStreamType)
				{
					case RtmpChunkStreamType::Type0:
					{
						this->receiveStreamID(chunkStreamType, maxSize, std::move(header), chunks, callback);
						break;
					}
						
					case RtmpChunkStreamType::Type1:
					{
						this->receivePayload(chunkStreamType, maxSize, std::move(header), chunks, callback);
						break;
					}
						
					default:
					{
						callback(ec, size, RtmpMediaPacketVariant());
						break;
					}
				}
			}
        }
        else
        {
            callback(ec, size, RtmpMediaPacketVariant());
        }
    });
}

void RtmpSocket::receiveExtentedTimestamp(RtmpChunkStreamType chunkStreamType, u32 maxSize, v8&& readData,
    RtmpChunk& chunks, const std::function<void(const boost::system::error_code& ec,
		std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback)
{
	receiveData(sizeof(u32), [header = std::forward<v8>(readData), chunkStreamType, maxSize, &chunks, callback, this]
				(const auto& ec, auto size, auto data) mutable
    {
		if (!ec)
		{
			std::move(data->begin(), data->end(), std::back_inserter(header));
			
			switch (chunkStreamType)
			{
				case RtmpChunkStreamType::Type0:
				{
					this->receiveStreamID(chunkStreamType, maxSize, std::move(header), chunks, callback);
					break;
				}
					
				case RtmpChunkStreamType::Type1:
				{
					this->receivePayload(chunkStreamType, maxSize, std::move(header), chunks, callback);
					break;
				}
					
				default:
				{
					callback(ec, size, RtmpMediaPacketVariant());
					break;
				}
			}
		}
		else
		{
			callback(ec, size, RtmpMediaPacketVariant());
		}
	});
}

void RtmpSocket::receiveStreamID(RtmpChunkStreamType chunkStreamType, u32 maxSize, v8&& readData, RtmpChunk& chunks,
	const std::function<void(const boost::system::error_code& ec,
		std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback)
{
	receiveData(sizeof(u32), [header = std::forward<v8>(readData), chunkStreamType, maxSize, &chunks, callback, this]
				(const auto& ec, auto size, auto data) mutable
    {
		if (!ec)
		{
			std::move(data->begin(), data->end(), std::back_inserter(header));
			this->receivePayload(chunkStreamType, maxSize, std::move(header), chunks, callback);
		}
		else
		{
			callback(ec, size, RtmpMediaPacketVariant());
		}
	});
}

void RtmpSocket::receivePayload(RtmpChunkStreamType chunkStreamType, u32 maxSize, v8&& header, RtmpChunk& chunks,
    const std::function<void(const boost::system::error_code& ec,
        std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback)
{
    auto &totalSize = std::get<1>(chunks);
    auto &currentSize = std::get<2>(chunks);
	auto variant = RtmpMediaPacketVariant{};
	
	switch (chunkStreamType)
	{
		case RtmpChunkStreamType::Type0:
		case RtmpChunkStreamType::Type1:
		{
			auto mediaPacket = RtmpPacketGenerator::newPacket<RtmpMediaMessage>();
			mediaPacket.deserialize(header.cbegin(), header.cend());
			
            totalSize = mediaPacket.payloadSize();
            currentSize = totalSize > maxSize ? maxSize : totalSize;
			const auto messageType = mediaPacket.type();
			
			switch (messageType)
			{
				case RtmpMessageType::VideoMessage:
				{
					auto packet = RtmpPacketGenerator::newPacket<RtmpVideoMessage>();
					packet.setChunkStreamType(mediaPacket.chunkStreamType());
					packet.deserialize(header.cbegin(), header.cend());
					variant = packet;
					break;
				}
					
				case RtmpMessageType::AudioMessage:
				{
					auto packet = RtmpPacketGenerator::newPacket<RtmpAudioMessage>();
					packet.setChunkStreamType(mediaPacket.chunkStreamType());
					packet.deserialize(header.cbegin(), header.cend());
					variant = packet;
					break;
				}
					
				case RtmpMessageType::DataMessage_AMF0:
				{
					auto packet = RtmpPacketGenerator::newPacket<RtmpMetadataMessage>();
					packet.setChunkStreamType(mediaPacket.chunkStreamType());
					packet.deserialize(header.cbegin(), header.cend());
					variant = packet;
					break;
				}
					
				default:
				{
					callback(boost::asio::error::invalid_argument, header.size(), std::move(variant));
					return;
				}
			}
		}
			
		default:
		{
			break;
		}
	}

    receiveData(currentSize, [p = std::move(variant), &chunks, callback, this]
		(const auto& ec, auto size, auto data) mutable
    {
        if (!ec)
        {
            auto payload = v8{};
            std::move(data->begin(), data->end(), std::back_inserter(payload));
            
            if (auto *packet1 = boost::get<RtmpAudioMessage>(&p))
            {
                packet1->setPayload(std::move(payload));
                packet1->deserialize();
            }
            else if (auto *packet2 = boost::get<RtmpVideoMessage>(&p))
            {
                packet2->setPayload(std::move(payload));
                packet2->deserialize();
            }
            else if (auto *packet3 = boost::get<RtmpMetadataMessage>(&p))
            {
                packet3->setPayload(std::move(payload));
                packet3->deserialize();
            }
            else  // variant has not been initialized, assuming it's a continuation
            {
                auto &ssize = std::get<2>(chunks);
				ssize = static_cast<u32>(payload.size());
                
                auto &ddata = std::get<v8>(chunks);
                std::move(payload.begin(), payload.end(), std::back_inserter(ddata));
            }
            
            callback(ec, size, std::move(p));
        }
        else
        {
            callback(ec, size, RtmpMediaPacketVariant());
        }
    });
}
