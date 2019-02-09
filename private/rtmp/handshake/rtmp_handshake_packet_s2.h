//
//  rtmp_handshake_packet_s1.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/handshake/rtmp_handshake_packet_c1.h"

namespace RtmpKit
{
	class RtmpHandshakePacketS2 : public RtmpHandshakePacketC1
	{
	public:
		RtmpHandshakePacketS2() = default;
		explicit RtmpHandshakePacketS2(const RtmpHandshakePacketC1& cp);
	};
} // end RtmpKit