//
//  mediatag.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../private/rtmp/rtmp_packet.h"

namespace RtmpKit
{
	class MediaTag : public RtmpPacket
	{
	public:
		MediaTag() = default;

		MediaTag(const MediaTag& cp) = default;

		MediaTag(MediaTag&& mv) = default;

		MediaTag& operator=(const MediaTag& cp) = default;

		MediaTag& operator=(MediaTag&& mv) = default;

		virtual ~MediaTag() = default;

		const v8 &payload() const;

		virtual void setPayload(const v8& p);

		virtual void setPayload(v8&& p);

		virtual const v8& serialize() override;

		virtual std::size_t size() const override;

		virtual u24 payloadSize() const = 0;

		virtual void setPayloadSize(u24 s) = 0;

		virtual void appendPayload(v8&& p) = 0;

		virtual const v8& data() const = 0;

	protected:
		v8 payload_;
	};
} // end rtmpkit
