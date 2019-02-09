//
//  rtmp_time.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 26/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include <chrono>

namespace RtmpKit
{
	using namespace std::chrono_literals;
	using CurrentClock = std::chrono::system_clock;
	using TimePoint = std::chrono::time_point<CurrentClock>;
	using TimeUnit = std::chrono::milliseconds;

	static RtmpKit::TimePoint currentTime()
	{
		auto now = CurrentClock::now();
		return now;
	}

	static TimeUnit currentTimeMillis()
	{
		auto now = RtmpKit::currentTime();
		auto duration = now.time_since_epoch();
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
		return milliseconds;
	}
} // end RtmpKit
