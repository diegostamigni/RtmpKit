//
//  flv_video_mediatag.h
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
	class FLVMediaVideoTag: public FLVMediaTag
	{
	public:
		FLVMediaVideoTag() = default;

		FLVMediaVideoTag(const FLVMediaVideoTag& cp) = default;

		FLVMediaVideoTag(FLVMediaVideoTag&& mv) = default;

		FLVMediaVideoTag& operator=(const FLVMediaVideoTag& cp) = default;

		FLVMediaVideoTag& operator=(FLVMediaVideoTag&& mv) = default;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		virtual void deserialize() override;

		virtual const v8& serialize() override;

		void updateMedataData();

		virtual FLVVideoCodecType codecId() const
		{
			return codecID_;
		}

		virtual void setCodecId(FLVVideoCodecType value)
		{
			codecID_ = value;
		}

		virtual FLVVideoFrameType frameType() const
		{
			return frameType_;
		}

		virtual void setFrameType(FLVVideoFrameType value)
		{
			frameType_ = value;
		}

	private:
		FLVVideoCodecType codecID_;
		FLVVideoFrameType frameType_;
	};
} // end RtmpKit