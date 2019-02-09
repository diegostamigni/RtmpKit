#include "../../../private/rtmp/handshake/rtmp_handshake_packet_c0.h"
#include "../../../private/rtmp/rtmp_utils.h"
using namespace RtmpKit;

RtmpHandshakePacketC0::RtmpHandshakePacketC0()
	: RtmpHandshakePacket(), version_(RtmpProtocolVersion), data_(1, version_)
{
}

const v8& RtmpHandshakePacketC0::serialize()
{
	data_.clear();
	data_.push_back(version_);
	return data_;
}

void RtmpHandshakePacketC0::deserialize()
{
}

void RtmpHandshakePacketC0::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	data_.clear();

	if (begin == end)
	{
		return;
	}

	version_ = *begin;
	data_.push_back(version_);
}

size_t RtmpHandshakePacketC0::size() const
{
	return RtmpHandshakePacketC0::defaultSize();
}

size_t RtmpHandshakePacketC0::defaultSize()
{
	return 1;
}
