//
//  rtmp_handshake_packet_c1.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/rtmp_time.h"
#include "../../../private/rtmp/handshake/rtmp_handshake_packet.h"

namespace RtmpKit
{
	class RtmpHandshakePacketC1 : public RtmpHandshakePacket
	{
	public:
		RtmpHandshakePacketC1();
		RtmpHandshakePacketC1(const RtmpHandshakePacketC1&) = default;
		RtmpHandshakePacketC1(RtmpHandshakePacketC1&&) = default;

		virtual const v8& serialize() override;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual std::size_t size() const override;

		static std::size_t defaultSize();

		RtmpKit::TimeUnit timestamp() const { return epoc_; }

		const v8& time() const { return time_; }

		const v8& zeros() const { return zeros_; }

		const v8& randomData() const { return randomBytes_; }

	protected:
		virtual void deserialize() override;

	protected:
		v8 data_;
		RtmpKit::TimeUnit epoc_;
		v8 time_;
		v8 zeros_;
		v8 randomBytes_;
	};
} // end RtmpKit
