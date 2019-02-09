//
//  rtmp_set_peer_bandwidth_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 4/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/control/rtmp_window_ack_size_message.h"

namespace RtmpKit
{
	class RtmpSetPeerBandWidthMessage : public RtmpWindowAcknowledgementSizeMessage
	{
	public:
		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		static std::size_t defaultSize();

		RtmpPeerBandWidthLimitType limitType() const
		{
			return limitType_;
		}

		void setLimitType(RtmpPeerBandWidthLimitType value)
		{
			limitType_ = value;
		}

	private:
		RtmpPeerBandWidthLimitType limitType_ = RtmpPeerBandWidthLimitType::Dynamic;
	};
} // end RtmpKit
