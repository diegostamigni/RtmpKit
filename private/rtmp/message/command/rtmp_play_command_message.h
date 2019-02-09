//
//  rtmp_play_command_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 24/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/command/rtmp_command_message.h"

namespace RtmpKit
{
	/*!
	* Represent the RTMP NetStream command message
	* @details 7.2.2.1. Adobe RTMP specification doc
	*/
	class RtmpPlayCommandMessage : public RtmpCommandMessage
	{
	public:
		RtmpPlayCommandMessage();

		explicit RtmpPlayCommandMessage(const std::string& publishingName);

		virtual const v8& serialize() override;

		const std::string& streamName() const
		{
			return streamName_;
		}

		void setStreamName(const std::string& value)
		{
			streamName_ = value;
		}

	private:
		std::string streamName_;
	};
} // end RtmpKit
