//
//  bufferable.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 03/02/2017.
//  Copyright © 2017 RtmpKit. All rights reserved.
//

#pragma once

#include "../private/utils.h"

#include <memory>

namespace RtmpKit
{
	class Bufferable
	{
	public:
		virtual ~Bufferable() = default;

		virtual RtmpKit::u24 timestamp() const = 0;
		virtual RtmpKit::u24 duration() const = 0;
		virtual RtmpKit::u32 bufferLenght() const = 0;
		virtual std::shared_ptr<RtmpKit::u8> payload() const = 0;
	};
}
