#include "../../../private/logger.h"
#include "../../../private/rtmp/session/rtmp_client_session.h"
#include "../../../private/rtmp/handshake/rtmp_handshake_packet_generator.h"
#include "../../../private/rtmp/handshake/rtmp_handshake_packet_c0.h"
#include "../../../private/rtmp/handshake/rtmp_handshake_packet_c2.h"
#include "../../../private/rtmp/handshake/rtmp_handshake_packet_s0.h"
#include "../../../private/rtmp/handshake/rtmp_handshake_packet_s2.h"
using namespace RtmpKit;

void RtmpClientSession::handshake()
{
	RtmpSession::handshake();
	sendC0();
}

void RtmpClientSession::sendC0()
{
	LOG_DEBUG("Sending handshake C0 part");
	auto packet = RtmpHandshakePacketGenerator::newPacket<RtmpHandshakePacketC0>();
	socket_->sendPacket(packet, [this](const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("C0 packet sent");
			this->sendC1();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send handshake C0 part: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpClientSession::sendC1()
{
	LOG_DEBUG("Sending handshake C1 part");
	auto packet = RtmpHandshakePacketGenerator::newPacket<RtmpHandshakePacketC1>();

	socket_->sendPacket(packet, [this](const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("C1 packet sent");
			this->receiveS0();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send handshake C1 part: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpClientSession::sendC2(const RtmpHandshakePacketS1 &s1)
{
	LOG_DEBUG("Sending handshake C2 part");
	auto packet = RtmpHandshakePacketGenerator::newPacket<RtmpHandshakePacketC2>(s1);

	socket_->sendPacket(packet, [this](const auto &ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("C2 packet sent");
			this->receiveS2();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send handshake C2 part: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpClientSession::receiveS0()
{
	LOG_DEBUG("Receiving handshake S0 part");
	socket_->receivePacket<RtmpHandshakePacketS0>([this](const auto& ec, auto size, auto &&data)
	{
		if (!ec)
		{
			LOG_DEBUG("S0 packet received");
			const auto sVersion = data.version();

			if (sVersion == this->version()) // if they are the same, let's keep going
			{
				this->receiveS1();
			}
			else
			{
				LOG_ERROR(str(boost::format("RtmpClientVersion(%1%) is not equal to the server version (%2%)")
					% this->version()
					% sVersion).c_str());
			}
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive packet S0: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpClientSession::receiveS1()
{
	LOG_DEBUG("Receiving handshake S1 part");
	socket_->receivePacket<RtmpHandshakePacketS1>([this](const auto& ec, auto size, auto &&data)
	{
		if (!ec)
		{
			LOG_DEBUG("S1 packet received");
			this->sendC2(data);
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive packet S1: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpClientSession::receiveS2()
{
	LOG_DEBUG("Receiving handshake S2 part");
	socket_->receivePacket<RtmpHandshakePacketS2>([this](const auto& ec, auto size, auto &&data)
	{
		if (!ec)
		{
			LOG_DEBUG("S2 packet received");
			LOG_INFO("Handshake done");
			this->handshakeCompleted();
			this->multicastDelegate_([](auto d){ d->handshakeDone(); });
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive packet S2: %1%") % ec.message()).c_str());
		}
	});
}
