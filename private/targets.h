//
//  targets.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 21/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

namespace RtmpKit
{
	enum class TargetType : int
	{
		Unknown = 0,
		iOS,
		Android,
		macOS,
		Windows,
		Linux,
	};

	TargetType currentTargetType();
}// end RtmpKit
