//
//  parser.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../private/utils.h"

namespace RtmpKit
{
	class Parsable
	{
	public:
		virtual ~Parsable() = default;
		virtual void deserialize() = 0;
		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) = 0;
	};
}
