
//
//  audiodescriptor.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 05/04/2017.
//  Copyright © 2017 RtmpKit. All rights reserved.
//

#pragma once

#include "../private/utils.h"

namespace RtmpKit
{	
	class AudioDescriptor
	{
	public:
		AudioDescriptor() = default;
		AudioDescriptor(const AudioDescriptor&) = default;
		AudioDescriptor(AudioDescriptor&&) = default;
		virtual ~AudioDescriptor() = default;
		
		AudioDescriptor& operator=(const AudioDescriptor&) = default;
		AudioDescriptor& operator=(AudioDescriptor&&) = default;

		int channelCount() const;
		
		void setChannelCount(int value);
		
		int frameLength() const;
		
		void setFrameLength(int value);
		
		int sampleRate() const;
		
		void setSampleRate(int value);
		
		bool isValid() const;

	public:
		int channelCount_ = 0;
		int frameLength_ = 0;
		int sampleRate_ = 0;
	};
} // end RtmpKit
