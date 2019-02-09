//
//  flv_audio_mediatag.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/flv/flv_mediatag.h"
#include "../../private/flv/flv_types.h"

namespace RtmpKit
{
	class FLVMediaAudioTag: public FLVMediaTag
	{
	public:
		FLVMediaAudioTag() = default;

		FLVMediaAudioTag(const FLVMediaAudioTag& cp) = default;

		FLVMediaAudioTag(FLVMediaAudioTag&& mv) = default;

		FLVMediaAudioTag& operator=(const FLVMediaAudioTag& cp) = default;

		FLVMediaAudioTag& operator=(FLVMediaAudioTag&& mv) = default;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual void deserialize() override;

		virtual const v8& serialize() override;

		void updateMedataData();

		virtual FLVSoundFormatType soundFormat() const
		{
			return soundFormat_;
		}

		virtual void setSoundFormat(FLVSoundFormatType format)
		{
			this->soundFormat_ = format;
		}

		virtual FLVSoundRateType soundRate() const
		{
			return soundRate_;
		}

		virtual void setSoundRate(FLVSoundRateType sound_rate)
		{
			soundRate_ = sound_rate;
		}

		virtual FLVSoundSampleSizeType soundSampleSize() const
		{
			return soundSampleSize_;
		}

		virtual void setSoundSampleSize(FLVSoundSampleSizeType sound_sample_size)
		{
			soundSampleSize_ = sound_sample_size;
		}

		virtual FLVSoundType soundType() const
		{
			return soundType_;
		}

		virtual void setSoundType(FLVSoundType sound_type)
		{
			soundType_ = sound_type;
		}

		virtual FLVAACPacketType aacPacketType() const
		{
			return aacPacketType_;
		}

		virtual void setAACPacketType(FLVAACPacketType aac_packet_type)
		{
			aacPacketType_ = aac_packet_type;
		}

	private:
		FLVSoundFormatType soundFormat_;
		FLVSoundRateType soundRate_;
		FLVSoundSampleSizeType soundSampleSize_;
		FLVSoundType soundType_;
		FLVAACPacketType aacPacketType_;
	};
} // end RtmpKit