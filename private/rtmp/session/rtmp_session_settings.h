//
//  rtmp_session_settings.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 6/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/flv/flv_types.h"

namespace RtmpKit
{
	class RtmpSessionSettings
	{
	public:
		RtmpSessionSettings();

		RtmpSessionSettings(const RtmpSessionSettings&) = default;

		RtmpSessionSettings(RtmpSessionSettings&&) = default;

		RtmpSessionSettings& operator=(const RtmpSessionSettings&) = default;

		RtmpSessionSettings& operator=(RtmpSessionSettings&&) = default;

		FLVVideoCodecType videoCodecId() const
		{
			return videoCodecID_;
		}

		void setVideoCodecId(FLVVideoCodecType value)
		{
			videoCodecID_ = value;
		}

		FLVSoundFormatType soundFormat() const
		{
			return soundFormat_;
		}

		void setSoundFormat(FLVSoundFormatType format)
		{
			this->soundFormat_ = format;
		}

		FLVSoundRateType soundRate() const
		{
			return soundRate_;
		}

		void setSoundRate(FLVSoundRateType sound_rate)
		{
			soundRate_ = sound_rate;
		}

		FLVSoundSampleSizeType soundSampleSize() const
		{
			return soundSampleSize_;
		}

		void setSoundSampleSize(FLVSoundSampleSizeType sound_sample_size)
		{
			soundSampleSize_ = sound_sample_size;
		}

		FLVSoundType soundType() const
		{
			return soundType_;
		}

		void setSoundType(FLVSoundType sound_type)
		{
			soundType_ = sound_type;
		}

		u32 chunkSize() const
		{
			return chunkSize_;
		}

		void setChunkSize(u32 value)
		{
			chunkSize_ = value;
		}

		double frameRate() const
		{
			return frameRate_;
		}

		void setFrameRate(double value)
		{
			frameRate_ = value;
		}

		double audioChannels() const
		{
			return audioChannels_;
		}

		void setAudioChannels(double value)
		{
			audioChannels_ = value;
		}

		double videoDataRate() const
		{
			return videoDataRate_;
		}

		void setVideoDataRate(double value)
		{
			videoDataRate_ = value;
		}

		double videoFrameRate() const
		{
			return videoFrameRate_;
		}

		void setVideoFrameRate(double value)
		{
			videoFrameRate_ = value;
		}

		double width() const
		{
			return width_;
		}

		void setWidth(double width)
		{
			width_ = width;
		}

		double height() const
		{
			return height_;
		}

		void setHeight(double height)
		{
			height_ = height;
		}

	private:
		u32 chunkSize_;
		FLVVideoCodecType videoCodecID_;
		FLVSoundFormatType soundFormat_;
		FLVSoundRateType soundRate_;
		FLVSoundSampleSizeType soundSampleSize_;
		FLVSoundType soundType_;
		double frameRate_;
		double audioChannels_;
		double videoDataRate_;
		double videoFrameRate_;
		double width_;
		double height_;
	};
} // end RtmpKit
