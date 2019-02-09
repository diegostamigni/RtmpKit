//
//  rtmp_command_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 29/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/rtmp_message.h"

namespace RtmpKit
{
	class RtmpCommandMessage : public RtmpMessage
	{
	public:
		const std::string& commandName() const
		{
			return commandName_;
		}

		void setCommandName(const std::string& value)
		{
			commandName_ = value;
		}

		u8 transactionID() const
		{
			return transactionID_;
		}

		void setTransactionID(u8 value)
		{
			transactionID_ = value;
		}

	protected:
		std::string commandName_;
		u8 transactionID_ = 0;
	};
} // end RtmpKit
