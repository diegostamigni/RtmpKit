//
//  rtmp_window_ack_size_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 3/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/rtmp_message.h"

namespace RtmpKit
{
	class RtmpWindowAcknowledgementSizeMessage : public RtmpMessage
	{
	public:
		RtmpWindowAcknowledgementSizeMessage();

		RtmpWindowAcknowledgementSizeMessage(u32 windowAcknowledgementSize);

		virtual const v8& serialize() override;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual std::size_t size() const override;

		static std::size_t defaultSize();

		void setWindowAcknowledgementSize(u32 value)
		{
			windowAcknowledgementSize_ = value;
		}

		u32 windowAcknowledgementSize() const
		{
			return windowAcknowledgementSize_;
		}

	private:
		u32 windowAcknowledgementSize_ = 0;
	};
} // end RtmpKit
