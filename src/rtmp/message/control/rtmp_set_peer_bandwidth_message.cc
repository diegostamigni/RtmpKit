#include "../../../../private/rtmp/message/control/rtmp_set_peer_bandwidth_message.h"
using namespace RtmpKit;

void RtmpSetPeerBandWidthMessage::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	RtmpWindowAcknowledgementSizeMessage::deserialize(begin, end);

	setLimitType(static_cast<RtmpPeerBandWidthLimitType>(payload_.back()));
}

std::size_t RtmpSetPeerBandWidthMessage::defaultSize()
{
	return sizeof(u32) + sizeof(RtmpPeerBandWidthLimitType) + 12 /* header size */;
}
