//
//  rtmp_handshake_packet_c1.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/handshake/rtmp_handshake_packet.h"
#include "../../../private/rtmp/handshake/rtmp_handshake_packet_s1.h"

namespace RtmpKit
{
	class RtmpHandshakePacketC2 : public RtmpHandshakePacketS1
	{
	public:
		RtmpHandshakePacketC2(const RtmpHandshakePacketS1 &cp);

		virtual const v8& serialize() override;
	};
} // end RtmpKit
