//
//  rtmp_close_stream_command_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 12/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/command/rtmp_command_message.h"

namespace RtmpKit
{
	class RtmpCloseStreamCommandMessage : public RtmpCommandMessage
	{
	public:
		RtmpCloseStreamCommandMessage();

		virtual const v8& serialize() override;
	};
} // end RtmpKit
