//
//  flv_mediatag.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/typed_tag.h"

namespace RtmpKit
{
	enum class FLVTagType : u8
	{
		Unknown = 0x00,
		AudioPayload = 0x08,
		VideoPayload = 0x09,
		AMFMetadata = 0x12,
	};

	class FLVMediaTag : public MediaTypedPacketTag<FLVTagType>
	{
	public:
		FLVMediaTag() = default;

		FLVMediaTag(const FLVMediaTag& cp) = default;

		FLVMediaTag(FLVMediaTag&& mv) = default;

		virtual FLVMediaTag& operator=(const FLVMediaTag& cp) = default;

		virtual FLVMediaTag& operator=(FLVMediaTag&& mv) = default;

		virtual void setPreviousTagSize(u32 s)
		{
			this->prevTagSize_ = s;
		}

		virtual u32 previousTagSize() const
		{
			return this->prevTagSize_;
		}

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual u24 payloadSizeAndHeader() const
		{
			// The entire packet size is given by it's metadata size + the payload size

			return payloadSize() + 15;
		}

	protected:
		virtual void deserialize() override;

	private:
		u32 prevTagSize_ = 0;
	};
} // end rtmpkit