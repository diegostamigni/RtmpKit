//
//  mediaitem.h
//
//  Created by Diego Stamigni on 12/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../private/mediaheader.h"
#include "../private/mediatag.h"
#include "parsable.h"

#include <vector>
#include <iterator>

namespace RtmpKit
{
	class MediaItem : public Parsable
	{
	public:
		virtual ~MediaItem() = default;

		virtual void appendPayload(v8&& p) = 0;
	};
} //end RtmpKit
