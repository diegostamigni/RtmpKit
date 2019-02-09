//
//  rtmp_publish_command_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 5/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/command/rtmp_command_message.h"

namespace RtmpKit
{
	/*!
	* Represent the RTMP NetStream command message
	* @details 7.2.2.6. Adobe RTMP specification doc
	*/
	class RtmpPublishCommandMessage : public RtmpCommandMessage
	{
	public:
		RtmpPublishCommandMessage();

		explicit RtmpPublishCommandMessage(const std::string& publishingName);

		virtual const v8& serialize() override;

		const std::string& publishingName() const
		{
			return publishingName_;
		}

		void setPublishingName(const std::string& value)
		{
			publishingName_ = value;
		}

		const std::string& publishingType() const
		{
			return publishingType_;
		}

		void setPublishingType(const std::string& value)
		{
			publishingType_ = value;
		}

	private:
		std::string publishingName_;
		std::string publishingType_;
	};
} // end RtmpKit
