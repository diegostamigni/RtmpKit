//
//  flv_player.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 12/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/player.h"

namespace RtmpKit
{
	class FLVPlayer : public Player
	{
	public:
		virtual void play() override;

		virtual void pause() override;

		virtual void stop() override;

		virtual void resume() override;
	};
} // end RtmpKit