//
//  player.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 12/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

namespace RtmpKit
{
	class Player
	{
	public:
		virtual ~Player() = default;

		virtual void play() = 0;

		virtual void pause() = 0;

		virtual void stop() = 0;

		virtual void resume() = 0;
	};
} // end RtmpKit