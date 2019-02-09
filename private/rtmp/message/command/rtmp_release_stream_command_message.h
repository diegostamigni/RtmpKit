//
//  rtmp_release_stream_command_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 18/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/command/rtmp_command_message.h"

namespace RtmpKit
{
	class RtmpReleaseStreamCommandMessage : public RtmpCommandMessage
	{
	public:
		RtmpReleaseStreamCommandMessage();

		explicit RtmpReleaseStreamCommandMessage(const std::string& publishingName);

		virtual const v8& serialize() override;

		const std::string& publishingName() const
		{
			return publishingName_;
		}

		void setPublishingName(const std::string& value)
		{
			publishingName_ = value;
		}

	private:
		std::string publishingName_;
	};
} // end RtmpKit
