//
//  rtmp_metadata_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 6/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/message/rtmp_media_message.h"
#include "../../../private/flv/flv_amf_mediatag.h"

namespace RtmpKit
{
	class RtmpMetadataMessage : public RtmpMediaMessage
	{
	public:
		RtmpMetadataMessage();

		explicit RtmpMetadataMessage(const FLVMediaAMFTag& metadata);

		virtual const v8& serialize() override;

		virtual std::size_t size() const override;

		virtual void deserialize() override;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

	public:
		double width() const;

		void setWidth(double width);

		double height() const;

		void setHeight(double height);

		double videoDataRate() const;

		void setVideoDataRate(double videodatarate);

		double videoFrameRate() const;

		void setVideoFrameRate(double value);

		double frameRate() const;

		void setFrameRate(double framerate);

		const std::string& videoCodecId() const;

		void setVideoCodecId(const std::string& value);

		double audioDataRate() const;

		void setAudioDataRate(double audiodatarate);

		double audioSampleRate() const;

		void setAudioSampleRate(double audiosamplerate);

		double audioChannels() const;

		void setAudioChannels(double value);

		const std::string& audioCodecId() const;

		void setAudioCodecId(const std::string& value);

	private:
		FLVMediaAMFTag metadata_;
	};
} // end RtmpKit
