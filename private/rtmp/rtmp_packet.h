//
//  rtmp_packet.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/utils.h"
#include "../../private/parsable.h"

namespace RtmpKit
{
	class RtmpPacket : public Parsable
	{
	public:
		virtual const v8& serialize() = 0;

		virtual std::size_t size() const = 0;
	};
} // end RtmpKit
