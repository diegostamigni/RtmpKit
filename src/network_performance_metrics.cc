#include "../private/network_performance_metrics.h"
#include "../private/logger.h"

using namespace RtmpKit;
using namespace std::chrono_literals;

static constexpr const int NetworkErrorLimit = 5;
static constexpr const int ByteRatePercentageDrop = 10;
static constexpr const int LoopCheckInSeconds = 1;
static const std::chrono::milliseconds StableNetworkTrigger = 30s;

NetworkPerformanceMetrics::NetworkPerformanceMetrics(std::size_t byteRate)
    : byteRate_(byteRate)
	, oldCbs_(0)
	, stableNetCounter_(0s)
	, networkErrorCounter_(0)
	, interval_(boost::posix_time::seconds(LoopCheckInSeconds))
    , timer_(io_service_)
    , stopped_(true)
    , currentNetwork_(NetworkType::Highest)
{
}

NetworkPerformanceMetrics::~NetworkPerformanceMetrics()
{
	stop();
}

void NetworkPerformanceMetrics::start()
{
    if (!stopped_) return;
    stopped_ = false;
	clear();
	timer_.expires_from_now(boost::posix_time::seconds(interval_));
	timer_.async_wait(std::bind(&NetworkPerformanceMetrics::run, this, std::placeholders::_1));
    thread_ = std::thread([io = &io_service_]() { io->run(); });
}

void NetworkPerformanceMetrics::stop()
{
    if (stopped_) return;
	stopped_ = true;
    timer_.cancel();
	
    if (std::this_thread::get_id() != thread_.get_id())
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
    }
    else
    {
        if (thread_.joinable())
        {
            thread_.detach();
        }
	}
	
	io_service_.stop();
	io_service_.reset();
}

void NetworkPerformanceMetrics::clear()
{
	std::unique_lock<std::mutex> lk(mutex_, std::defer_lock);
	if (lk.try_lock())
	{
		oldCbs_ = 0;
		stableNetCounter_ = 0s;
		networkErrorCounter_ = 0;
		nsi_.reset();
	}
	lk.unlock();
}

void NetworkPerformanceMetrics::setByteRate(std::size_t value)
{
	byteRate_ = value;
}

void NetworkPerformanceMetrics::updateData(std::size_t data)
{
    std::lock_guard<std::mutex> _(mutex_);
    {
        nsi_.setTransferredBytes(data);
    }
}

void NetworkPerformanceMetrics::setDelegate(const std::shared_ptr<NetworkPerformanceMetricsDelegate> delegate)
{
    delegate_ = delegate;
}

void NetworkPerformanceMetrics::run(const boost::system::error_code&)
{
	if (stopped_)
	{
		timer_.cancel();
		return;
	}
	
    std::lock_guard<std::mutex> lk(mutex_);
    {
		const auto cbs = static_cast<std::size_t>(std::ceil(nsi_.currentBytesPerSeconds() / interval_.total_seconds()));
		
		if (cbs > 0)
		{
			handleNetworkProfiler(cbs);
		}
		
		timer_.expires_from_now(interval_);
		timer_.async_wait(std::bind(&NetworkPerformanceMetrics::run, this, std::placeholders::_1));
	}
}

void NetworkPerformanceMetrics::handleNetworkProfiler(std::size_t cbs)
{
	auto newNetworkType = currentNetwork_;
	auto newByteRate = byteRate_;
	
	if (byteRateShouldDrop(cbs) || cbs == oldCbs_)
	{
		if (cbs == oldCbs_ && networkErrorCounter_ < NetworkErrorLimit)
		{
			++networkErrorCounter_;
		}
		else
		{
			--newNetworkType;
			stableNetCounter_ = 0s; // reset the stable net counter
			networkErrorCounter_ = 0; // reset the net error counter
			newByteRate = byteRate_ - newByteRateFromPercentage();
		}
	}
	else if (stableNetCounter_ > StableNetworkTrigger)
	{
		++newNetworkType;
		stableNetCounter_ = 0s; // reset the stable net counter
		networkErrorCounter_ = 0; // reset the net error counter
		newByteRate = byteRate_ + newByteRateFromPercentage();
	}
	else
	{
		// stable connection, let's count for how long it's stable so that we can upgrade the current net type
		stableNetCounter_ += std::chrono::seconds(interval_.total_seconds());
	}
	
	notifyListenersIfNeeded(newNetworkType, newByteRate);
	currentNetwork_ = newNetworkType;
	oldCbs_ = cbs;
}

void NetworkPerformanceMetrics::notifyListenersIfNeeded(NetworkType newNetworkType, std::size_t newByteRate)
{
	if (auto ptr = delegate_.lock())
	{
		auto notify = false;
		
		if (newNetworkType > currentNetwork_)
		{
			ptr->networkMetricsUpgraded(*this, newNetworkType, newByteRate);
			notify = true;
		}
		else if (newNetworkType < currentNetwork_)
		{
			ptr->networkMetricsDowngraded(*this, newNetworkType, newByteRate);
			notify = true;
		}
		
		if (notify)
		{
			ptr->networkMetricsChanged(*this, newNetworkType, newByteRate);
		}
	}
}

float NetworkPerformanceMetrics::newByteRateFromPercentage() const
{
	return ((byteRate_ * ByteRatePercentageDrop) / 100);
}

bool NetworkPerformanceMetrics::byteRateShouldDrop(std::size_t cbs) const
{
	const auto limit = std::floor((byteRate_ * 1.0) / 3);
	const auto shouldDrop = cbs < limit;
	return shouldDrop;
}
