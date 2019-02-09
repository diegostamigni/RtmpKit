//
//  rtmp_set_chunk_size_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 4/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/control/rtmp_window_ack_size_message.h"

namespace RtmpKit
{
	class RtmpSetChunkSizeMessage : public RtmpMessage
	{
	public:
		RtmpSetChunkSizeMessage() = default;

		explicit RtmpSetChunkSizeMessage(u32 size);

		virtual const v8& serialize() override;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual std::size_t size() const override;

		static std::size_t defaultSize();

		u32 chunkSize() const
		{
			return chunkSize_;
		}

		void setChunkSize(u32 value)
		{
			chunkSize_ = value;
		}

	private:
		u32 chunkSize_;
	};
} // end RtmpKit
