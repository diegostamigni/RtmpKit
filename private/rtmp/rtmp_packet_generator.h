//
//  rtmp_packet_generator.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/rtmp/message/rtmp_media_message.h"

#include <type_traits>
#include <vector>

namespace RtmpKit
{
	class RtmpPacketGenerator
	{
	public:
		template<typename T, typename... Args,
			typename = std::enable_if<std::is_base_of<RtmpPacket, T>::value
				&& std::is_default_constructible<T>::value>>
		static auto newPacket(Args&&... args) -> T
		{
			return T(std::forward<Args>(args)...);
		}

		template<typename T, typename B,
			typename = std::enable_if<std::is_base_of<RtmpPacket, T>::value
				&& std::is_copy_constructible<B>::value
				&& std::is_constructible<T, B>::value>>
		static auto newPacket(const B &b) -> T
		{
			return T(b);
		}

		template<typename T, typename... Args,
			typename = std::enable_if<std::is_base_of<RtmpMediaMessage, T>::value
				&& std::is_default_constructible<T>::value>>
		static auto newChunkedPacket(u32 maxSize, Args&&... args) -> std::vector<T>
		{
			auto head = RtmpPacketGenerator::newPacket<T>(std::forward<Args>(args)...);
			auto chunks = std::vector<T>{};
			auto remainingData = std::move(head.trunkToSize(maxSize));
			chunks.push_back(std::move(head));

			for (auto&& data : remainingData)
			{
				chunks.emplace_back(head.chunkStreamFmt(), std::move(data));
			}

			return chunks;
		}
	};
} // end RtmpKit
