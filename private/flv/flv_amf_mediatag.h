//
//  flv_amf_mediatag.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/flv/flv_mediatag.h"
#include "../../private/amf/types.h"

#define FLV_AMF_ONMETADATA "onMetaData"
#define FLV_AMF_PROP_DURATION "duration"
#define FLV_AMF_PROP_WIDTH "width"
#define FLV_AMF_PROP_HEIGHT "height"
#define FLV_AMF_PROP_VIDEODATARATE "videodatarate"
#define FLV_AMF_PROP_VIDEOFRAMERATE "videoframerate"
#define FLV_AMF_PROP_FRAMERATE "framerate"
#define FLV_AMF_PROP_VIDEOCODECID "videocodecid"
#define FLV_AMF_PROP_AUDIODATARATE "audiodatarate"
#define FLV_AMF_PROP_AUDIOSAMPLERATE "audiosamplerate"
#define FLV_AMF_PROP_AUDIOSAMPLESIZE "audiosamplesize"
#define FLV_AMF_PROP_STEREO "stereo"
#define FLV_AMF_PROP_TIMESTAMP "timestamp"
#define FLV_AMF_PROP_AUDIOCODECID "audiocodecid"
#define FLV_AMF_PROP_AUDIOCHANNELS "audiochannels"
#define FLV_AMF_PROP_ENCODER "encoder"
#define FLV_AMF_PROP_FILESIZE "filesize"
#define FLV_AMF_PROP_AVCLEVEL "avclevel"
#define FLV_AMF_PROP_AVCPROFILE "avcprofile"

namespace RtmpKit
{	
	class FLVMediaAMFTag: public FLVMediaTag
	{
	public:
		FLVMediaAMFTag();

		FLVMediaAMFTag(const FLVMediaAMFTag& cp) = default;

		FLVMediaAMFTag(FLVMediaAMFTag&& mv) = default;

		FLVMediaAMFTag& operator=(const FLVMediaAMFTag& cp) = default;

		FLVMediaAMFTag& operator=(FLVMediaAMFTag&& mv) = default;

		virtual void deserialize() override;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual const v8& serialize() override;

		double duration() const
		{
			return duration_;
		}

		void setDuration(double duration)
		{
			duration_ = duration;
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

		double videoDataRate() const
		{
			return videodatarate_;
		}

		void setVideoDataRate(double videodatarate)
		{
			videodatarate_ = videodatarate;
		}

		double videoFrameRate() const
		{
			return videoframerate_;
		}

		void setVideoFrameRate(double value)
		{
			videoframerate_ = value;
		}

		double frameRate() const
		{
			return framerate_;
		}

		void setFrameRate(double framerate)
		{
			framerate_ = framerate;
		}

		double videoCodecId() const
		{
			return videocodecid_;
		}

		const std::string& videoCodecIdStr() const
		{
			return videocodecidStr_;
		}

		void setVideoCodecId(double videocodecid)
		{
			videocodecid_ = videocodecid;
		}

		void setVideoCodecIdStr(const std::string& value)
		{
			videocodecidStr_ = value;
		}

		double audioDataRate() const
		{
			return audiodatarate_;
		}

		void setAudioDataRate(double audiodatarate)
		{
			audiodatarate_ = audiodatarate;
		}

		double audioSampleRate() const
		{
			return audiosamplerate_;
		}

		void setAudioSampleRate(double audiosamplerate)
		{
			audiosamplerate_ = audiosamplerate;
		}

		double audioSampleSize() const
		{
			return audiosamplesize_;
		}

		void setAudioSampleSize(double audiosamplesize)
		{
			audiosamplesize_ = audiosamplesize;
		}

		bool stereo() const
		{
			return stereo_;
		}

		void setStereo(bool stereo)
		{
			stereo_ = stereo;
		}

		double audioCodecId() const
		{
			return audiocodecid_;
		}

		void setAudioCodecId(double audiocodecid)
		{
			audiocodecid_ = audiocodecid;
		}

		double audioChannels() const
		{
			return audiochannels_;
		}

		void setAudioChannels(double value)
		{
			audiochannels_ = value;
		}

		const std::string& audioCodecIdStr() const
		{
			return audiocodecidStr_;
		}

		void setAudioCodecIdStr(const std::string& value)
		{
			audiocodecidStr_ = value;
		}

		std::string encoder() const
		{
			return encoder_;
		}

		void setEncoder(const std::string& encoder)
		{
			encoder_ = encoder;
		}

		double fileSize() const
		{
			return filesize_;
		}

		void setFileSize(double filesize)
		{
			filesize_ = filesize;
		}
		
		double avcLevel() const
		{
			return avcLevel_;
		}
		
		void setAvcLevel(double value)
		{
			avcLevel_ = value;
		}
		
		double avcProfle() const
		{
			return avcProfile_;
		}
		
		void setAvcProfile(double value)
		{
			avcProfile_ = value;
		}

	protected:
		virtual void deserializeFromDict(const AMF::TDictSubset& propDict);

	private:
		double duration_ = 0;
		double width_ = 0;
		double height_ = 0;
		double videoframerate_ = 0;
		double videodatarate_ = 0;
		double framerate_ = 0;
		double videocodecid_ = 0;
		std::string videocodecidStr_;
		double audiodatarate_ = 0;
		double audiosamplerate_ = 0;
		double audiosamplesize_ = 0;
		bool stereo_ = false;
		double audiocodecid_ = 0;
		double audiochannels_ = 0;
		std::string audiocodecidStr_;
		std::string encoder_;
		double filesize_ = 0;
		double avcLevel_ = 0;
		double avcProfile_ = 0;
	};
} // end RtmpKit
