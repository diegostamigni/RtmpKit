//
//  rtmp_handshake_packet_c0.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/handshake/rtmp_handshake_packet.h"

namespace RtmpKit
{
	class RtmpHandshakePacketC0 : public RtmpHandshakePacket
	{
	public:
		RtmpHandshakePacketC0();

		virtual const v8& serialize() override;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual size_t size() const override;

		static std::size_t defaultSize();

		u8 version() const { return version_; }

	protected:
		virtual void deserialize() override;

	private:
		u8 version_;
		v8 data_;
	};
} // end RtmpKit