//
//  typed_tag.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 28/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../private/mediatag.h"
#include "../private/typified.h"
#include "../private/utils.h"

namespace RtmpKit
{
	template <typename Type>
	class MediaTypedPacketTag : public MediaTag, public Typified<Type>
	{
	public:
		u24 payloadSize() const override
		{
			return payloadSize_;
		}

		void setPayloadSize(u24 s) override
		{
			this->payloadSize_ = s;
		}

		virtual void appendPayload(v8&& data) override
		{
			if (data.empty()) return;
			std::move(data.begin(), data.end(), std::back_inserter(payload_));
		}

		virtual const v8& data() const override
		{
			return payload_;
		}

		u32 timestamp() const
		{
			return timestamp_;
		}

		void setTimestamp(u32 s)
		{
			this->timestamp_ = s;
		}

		u24 streamID() const
		{
			return streamID_;
		}

		void setStreamID(u24 s)
		{
			this->streamID_ = s;
		}
		
		u32 extendedTimestamp() const
		{
			return extendedTimestamp_;
		}
		
		void setExtendedTimestamp(u32 value)
		{
			extendedTimestamp_ = value;
		}
		
		bool hasExtendedTimestamp() const
		{
			return RtmpKit::numberOfSetBits(timestamp()) >= 24;
		}

	protected:
		u32 payloadSize_ = 0;
		u32 extendedTimestamp_ = 0;
		u32 timestamp_ = 0;
		u24 streamID_ = 0;
	};
};
