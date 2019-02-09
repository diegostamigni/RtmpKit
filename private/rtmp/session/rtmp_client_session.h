//
//  rtmp_socket.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 21/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/session/rtmp_session.h"
#include "../../../private/rtmp/handshake/rtmp_handshake_packet_s1.h"

namespace RtmpKit
{
	struct RtmpClientSessionDelegate : public RtmpSessionDelegate
	{
	};

	class RtmpClientSession : public RtmpSession
	{
	public:
		virtual void handshake() override;

	private:
		void sendC0();
		void sendC1();
		void sendC2(const RtmpHandshakePacketS1 &packet);

		void receiveS0();
		void receiveS1();
		void receiveS2();
	};
} // end RtmpKit
