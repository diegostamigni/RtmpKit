//
//  videodescriptor.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 05/04/2017.
//  Copyright © 2017 RtmpKit. All rights reserved.
//

#pragma once

#include "../private/utils.h"

namespace RtmpKit
{
	class VideoDescriptor
	{
	public:
		VideoDescriptor() = default;
		VideoDescriptor(const VideoDescriptor&) = default;
		VideoDescriptor(VideoDescriptor&&) = default;
		virtual ~VideoDescriptor() = default;
		
		VideoDescriptor& operator=(const VideoDescriptor&) = default;
		VideoDescriptor& operator=(VideoDescriptor&&) = default;
		
	public:
		const RtmpKit::v8& sps() const;
		
		void setSPS(const RtmpKit::v8& value);

		void setSPS(RtmpKit::v8&& value);

		const RtmpKit::v8& pps() const;
		
		void setPPS(const RtmpKit::v8& value);

		void setPPS(RtmpKit::v8&& value);

		int32_t nalUnitSize() const;
		
		void setNalUnitSize(int32_t value);
		
		bool isValid() const;
		
	private:
		RtmpKit::v8 sps_, pps_;
		int32_t nalUnitSize_ = 0;
	};
} // end RtmpKit
