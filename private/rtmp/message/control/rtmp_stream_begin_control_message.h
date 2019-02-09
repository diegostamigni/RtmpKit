//
//  rtmp_stream_begin_control_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 25/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/command/rtmp_command_message.h"
#include "../../../../private/rtmp/message/command/rtmp_command_response_message.h"

namespace RtmpKit
{
	class RtmpStreamBeginControlMessage : public RtmpCommandMessage, public RtmpCommandMessageResponse
	{
	public:
		RtmpStreamBeginControlMessage();

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual void deserialize() override;

		virtual size_t size() const override;

		static std::size_t headerSize();

		virtual bool isSuccessfull() const override;

		virtual std::string description() const override;
	};
} // end RtmpKit
