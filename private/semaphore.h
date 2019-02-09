//
//  semaphore.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 22/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include <condition_variable>
#include <mutex>

namespace RtmpKit
{
	class SimpleSemaphore {
	public:
		explicit SimpleSemaphore (int count_ = 0);

		void notify();
		void wait();

	private:
		std::mutex mtx;
		std::condition_variable cv;
		int count;
	};
}
