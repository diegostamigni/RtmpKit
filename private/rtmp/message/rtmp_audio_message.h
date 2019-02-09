//
//  rtmp_audio_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/message/rtmp_media_message.h"
#include "../../../private/flv/flv_audio_mediatag.h"

namespace RtmpKit
{
	class RtmpAudioMessage : public RtmpMediaMessage
	{
	public:
		RtmpAudioMessage() = default;

		explicit RtmpAudioMessage(u8 chunkStreamId, v8&& data);

		explicit RtmpAudioMessage(FLVMediaAudioTag&& tag);

		virtual const v8& serialize() override;

		virtual void deserialize() override;

		virtual size_t size() const override;

		FLVSoundFormatType soundFormat() const;

		void setSoundFormat(FLVSoundFormatType format);

		FLVSoundRateType soundRate() const;

		void setSoundRate(FLVSoundRateType sound_rate);

		FLVSoundSampleSizeType soundSampleSize() const;

		void setSoundSampleSize(FLVSoundSampleSizeType sound_sample_size);

		FLVSoundType soundType() const;

		void setSoundType(FLVSoundType sound_type);

		FLVAACPacketType aacPacketType() const;

		void setAACPacketType(FLVAACPacketType aac_packet_type);

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;
		
		virtual const v8& data() const override;
		
		virtual u32 delay() const override;
		
		virtual void appendPayload(v8&& data) override;

	private:
		FLVMediaAudioTag audioTag_;
	};
} // end RtmpKit
