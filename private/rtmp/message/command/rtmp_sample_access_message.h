//
//  rtmp_sample_access_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 24/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/command/rtmp_command_message.h"
#include "../../../../private/rtmp/message/command/rtmp_command_response_message.h"

namespace RtmpKit
{
	class RtmpSampleAccessCommandMessage : public RtmpCommandMessage, public RtmpCommandMessageResponse
	{
	public:
		RtmpSampleAccessCommandMessage() = default;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual void deserialize() override;

		virtual size_t size() const override;

		static std::size_t headerSize();

		virtual bool isSuccessfull() const override;

		virtual std::string description() const override;

		void setAudioAllowed(bool value)
		{
			isAudioAllowed_ = value;
		}

		bool isAudioAllowed() const
		{
			return isAudioAllowed_;
		}

		void setVideoAllowed(bool value)
		{
			isVideoAllowed_ = value;
		}

		bool isVideoAllowed() const
		{
			return isVideoAllowed_;
		}

	private:
		bool isAudioAllowed_ = false;
		bool isAudioAllowedSet_ = false;
		bool isVideoAllowed_ = false;
		bool isVideoAllowedSet_ = false;
	};
} // end RtmpKit
