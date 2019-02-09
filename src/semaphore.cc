#include "../private/semaphore.h"

using namespace RtmpKit;

SimpleSemaphore::SimpleSemaphore(int count_)
	: count(count_)
{
}

void SimpleSemaphore::notify()
{
	std::unique_lock<std::mutex> lock(mtx);
	count++;
	cv.notify_one();
}

void SimpleSemaphore::wait()
{
	std::unique_lock<std::mutex> lock(mtx);

	while(count == 0)
		cv.wait(lock);

	count--;
}
