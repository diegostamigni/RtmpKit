//
//  rtmp_video_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/message/rtmp_media_message.h"
#include "../../../private/flv/flv_video_mediatag.h"

namespace RtmpKit
{
	class RtmpVideoMessage : public RtmpMediaMessage
	{
	public:
		RtmpVideoMessage() = default;

		explicit RtmpVideoMessage(u8 chunkStreamId, v8&& data);

		explicit RtmpVideoMessage(FLVMediaVideoTag&& tag);

		virtual const v8& serialize() override;

		virtual void deserialize() override;

		virtual size_t size() const override;

		FLVVideoCodecType codecId() const;

		void setCodecId(FLVVideoCodecType value);

		FLVVideoFrameType frameType() const;

		void setFrameType(FLVVideoFrameType value);

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;
		
		virtual const v8& data() const override;
		
		virtual void appendPayload(v8&& data) override;
		
	private:
		FLVMediaVideoTag videoTag_;
	};
} // end RtmpKit
