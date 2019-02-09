//
//  rtmp_media_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 7/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/message/rtmp_message.h"

namespace RtmpKit
{
	class RtmpMediaMessage : public RtmpMessage
	{
	public:
		virtual std::vector<v8> trunkToSize(u32 size) override;
		
		virtual const v8& serialize() override;
		
		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

	protected:
		virtual void updatePayloadSize(u32 size);
		
		virtual void updateChunkStreamId(u8 fmt);
	};
} // end RtmpKit
