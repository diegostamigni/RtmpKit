//
//  mediautils.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 05/04/2017.
//  Copyright © 2017 RtmpKit. All rights reserved.
//

#pragma once

#include "../private/utils.h"
#include "../private/videodescriptor.h"
#include "../private/audiodescriptor.h"

#include <utility>

namespace RtmpKit
{
	class MediaUtils
	{
	public:
		MediaUtils() = delete;
		MediaUtils(const MediaUtils&) = delete;
		MediaUtils(MediaUtils&&) = delete;

		MediaUtils& operator=(const MediaUtils&) = delete;
		MediaUtils& operator=(MediaUtils&&) = delete;

	/* 
	 * Video stuff 
	 */
	public:
		/*!
		 * Unpack SPS and PPS from a payload in a range for a specifed timestamp.
		 * @param payload the data to unpack
		 * @param timestamp the timestamp of the payload
		 * @returns VideoDescriptor holding SPS, PPS and other stuff regarding the video format
		 */
		static VideoDescriptor unpackVideoData(RtmpKit::v8::const_iterator& begin,
											   RtmpKit::v8::const_iterator& end,
											   RtmpKit::u32& timestamp,
											   bool appendNaluHeader = false);
		
	/* 
	 * Audio stuff 
	 */
	public:
		
		/*!
		 * Unpack audio format from a payload in a range.
		 * @param payload the data to unpack
		 * @returns VideoDescriptor holding SPS, PPS and other stuff regarding the video format
		 */
		static AudioDescriptor unpackAudioData(RtmpKit::v8::const_iterator& begin,
											   RtmpKit::v8::const_iterator& end);
		
		/*!
		 * The supported audio saple rates
		 */
		static std::vector<int> audioSampleRates();
		
		/*!
		 * Returns the index for a given sample rate
		 */
		static int indexForSampleRate(int sampleRate);
	};
}
