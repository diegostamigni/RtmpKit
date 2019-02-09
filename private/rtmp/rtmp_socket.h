//
//  rtmp_socket.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 21/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/socket/base_socket.h"
#include "../../private/rtmp/rtmp_packet.h"
#include "../../private/rtmp/rtmp_packet_generator.h"
#include "../../private/rtmp/message/rtmp_audio_message.h"
#include "../../private/rtmp/message/rtmp_video_message.h"
#include "../../private/rtmp/message/rtmp_metadata_message.h"

#include <utility>
#include <type_traits>

namespace RtmpKit
{
    using RtmpMediaPacketVariant = Variant<boost::blank, RtmpMetadataMessage, RtmpAudioMessage, RtmpVideoMessage>;
    using RtmpChunk = std::tuple<v8, u32, u32>;

	class RtmpSocket : public BaseSocket
	{
	public:
		/*!
		 * Send an RtmpPacket throught the socket.
		 * @param packet the RTMP packet to send
		 * @param callback the callback to call to when the send has done, with a ref to the packet sent
		 */
		template<typename T,
			typename = std::enable_if<std::is_base_of<RtmpPacket, T>::value
				&& std::is_default_constructible<T>::value
				&& !std::is_const<T>::value>>
		void sendPacket(T& packet,
			const std::function<void(const boost::system::error_code& ec, std::size_t bytes_transferred)> &callback)
		{
			sendData(packet.serialize(), callback);
		}

		/*!
		 * Receive an RtmpPacket throught the socket.
		 * @param callback the callback to call to when the async send has done, with the packet received
		 */
		template<typename T,
			typename = std::enable_if<std::is_base_of<RtmpPacket, T>::value
				&& std::is_default_constructible<T>::value
				&& std::is_move_constructible<T>::value
				&& std::is_move_assignable<T>::value>>
		void receivePacket(const std::function<void(const boost::system::error_code& ec,
			std::size_t bytes_transferred, T&& packet)> &callback)
		{
			receiveData(T::defaultSize(), [callback](const auto& ec, auto size, auto data)
			{
				auto packet = RtmpPacketGenerator::newPacket<T>();
				packet.deserialize(data->cbegin(), data->cend());
				callback(ec, size, std::move(packet));
			});
		}

		/*!
		 * Receive an RtmpPacket dynamic sized packet throught the socket.
		 * @param callback the callback to call to when the send has done, with the packet received
		 */
		template<typename T,
			typename = std::enable_if<std::is_base_of<RtmpPacket, T>::value
				&& std::is_default_constructible<T>::value
				&& std::is_move_constructible<T>::value
				&& std::is_move_assignable<T>::value>>
		void receiveDynamicPacket(const std::function<void(const boost::system::error_code& ec,
			std::size_t bytes_transferred, T&& packet)> &callback)
		{
			receiveData(T::headerSize(), [callback, this](const auto& ec, auto size, auto data)
			{
				auto packet = RtmpPacketGenerator::newPacket<T>();
				packet.deserialize(data->cbegin(), data->cend());
				const auto payloadSize = packet.payloadSize();

				this->receiveData(payloadSize,
					[p = std::move(packet), callback](const auto& ec_, auto size_, auto data_) mutable
				{
					auto payload = v8{};
					std::move(data_->begin(), data_->end(), std::back_inserter(payload));

					p.setPayload(std::move(payload));
					p.deserialize();

					callback(ec_, size_, std::move(p));
				});
			});
		}

		/*!
		* Receive an RtmpPacket with a dynamic header and dynamic body throught the socket.
		* @param callback the callback to call to when the send has done, with the packet received
		*/
		void receiveDynamicPacketWithDynamicHeader(u32 maxSize,
            const std::function<void(const boost::system::error_code& ec,
                std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)> &callback);
        
    private:
        void receiveChunk(RtmpChunk& chunks, u32 maxSize, const std::function<void(const boost::system::error_code& ec,
            std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback);
        
        void receiveTimestamp(RtmpChunkStreamType chunkStreamType, u32 maxSize, v8&& readData, RtmpChunk& chunks,
            const std::function<void(const boost::system::error_code& ec,
                std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback);
        
        void receiveExtentedTimestamp(RtmpChunkStreamType chunkStreamType, u32 maxSize, v8&& readData, RtmpChunk& chunks,
            const std::function<void(const boost::system::error_code& ec,
                std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback);
        
        void receiveBodySize(RtmpChunkStreamType chunkStreamType, u32 maxSize, bool hasExtTimestamp, v8&& readData,
            RtmpChunk& chunks, const std::function<void(const boost::system::error_code& ec,
                std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback);
        
        void receiveMessageType(RtmpChunkStreamType chunkStreamType, u32 maxSize, bool hasExtTimestamp, v8&& readData,
            RtmpChunk& chunks, const std::function<void(const boost::system::error_code& ec,
				std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback);
		
		void receiveStreamID(RtmpChunkStreamType chunkStreamType, u32 maxSize, v8&& readData, RtmpChunk& chunks,
			const std::function<void(const boost::system::error_code& ec,
				std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback);
		
        void receivePayload(RtmpChunkStreamType chunkStreamType, u32 maxSize, v8&& readData, RtmpChunk& chunks,
            const std::function<void(const boost::system::error_code& ec,
                std::size_t bytes_transferred, RtmpMediaPacketVariant&& packet)>& callback);
	};
} // end RtmpKit
