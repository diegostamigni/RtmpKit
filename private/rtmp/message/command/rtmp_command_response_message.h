//
//  rtmp_command_response_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 5/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

namespace RtmpKit
{
	struct RtmpCommandMessageResponse
	{
		virtual ~RtmpCommandMessageResponse() = default;

		virtual bool isSuccessfull() const = 0;

		virtual std::string description() const = 0;
	};
} // end RtmpKit
