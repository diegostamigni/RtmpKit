#include "../private/network_speed_indicator.h"
#include "../private/logger.h"

using namespace RtmpKit;

NetworkSpeedIndicator::NetworkSpeedIndicator()
	: start_(std::chrono::system_clock::now())
	, currentBytes_(0)
{
}

void NetworkSpeedIndicator::setTransferredBytes(std::size_t value)
{
    current_ = std::chrono::system_clock::now();
    currentBytes_ += value;
}

std::size_t NetworkSpeedIndicator::currentBytesPerSeconds() const
{
    const auto startSec = std::chrono::time_point_cast<std::chrono::seconds>(start_);
    const auto endSec = std::chrono::time_point_cast<std::chrono::seconds>(current_);
	const auto diff = endSec.time_since_epoch().count() - startSec.time_since_epoch().count();
    return diff > 0 ? currentBytes_ / static_cast<std::size_t>(diff) : currentBytes_;
}

std::size_t NetworkSpeedIndicator::currentBitPerSeconds() const
{
	return static_cast<std::size_t>(std::ceil(currentBytesPerSeconds() * 8));
}

std::size_t NetworkSpeedIndicator::currentKilobytesPerSeconds() const
{
	return static_cast<std::size_t>(std::ceil(currentBytesPerSeconds() / 1000));
}

void NetworkSpeedIndicator::reset()
{
	currentBytes_ = 0;
	start_ = std::chrono::system_clock::now();
}
