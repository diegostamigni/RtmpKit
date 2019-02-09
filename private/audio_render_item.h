//
//  audio_render_item.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 03/02/2017.
//  Copyright © 2017 RtmpKit. All rights reserved.
//

#pragma once

#include "../private/bufferable.h"

namespace RtmpKit
{
	class AudioRenderItem : public Bufferable
	{
	public:
        explicit AudioRenderItem(RtmpKit::u24 timestamp, RtmpKit::u24 duration,
            RtmpKit::u32 bufferLenght, std::shared_ptr<RtmpKit::u8> payload);

        AudioRenderItem() = default;
        AudioRenderItem(const AudioRenderItem&) = default;
        AudioRenderItem(AudioRenderItem&&) = default;
		
		AudioRenderItem& operator=(const AudioRenderItem&) = default;
		AudioRenderItem& operator=(AudioRenderItem&&) = default;

		virtual ~AudioRenderItem() = default;

        void setTimestamp(RtmpKit::u24 value);
        void setDuration(RtmpKit::u24 value);
        void setBufferLenght(RtmpKit::u32 value);
        void setPayload(std::shared_ptr<RtmpKit::u8> value);

		virtual RtmpKit::u24 timestamp() const override
        {
            return timestamp_;
        }

		virtual RtmpKit::u24 duration() const override
        {
            return duration_;
        }

		virtual RtmpKit::u32 bufferLenght() const override
        {
            return bufferLenght_;
        }

		virtual std::shared_ptr<RtmpKit::u8> payload() const override
        {
            return payload_;
        }

    private:
        RtmpKit::u24 timestamp_;
        RtmpKit::u24 duration_;
        RtmpKit::u32 bufferLenght_;
        std::shared_ptr<RtmpKit::u8> payload_;
	};
} // end RtmpKit
