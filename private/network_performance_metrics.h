//
//  network_performance_metrics.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 7/11/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../include/network_types.h"

#include "../private/network_speed_indicator.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <memory>
#include <chrono>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace RtmpKit
{
	class NetworkPerformanceMetrics;
		
	struct NetworkPerformanceMetricsDelegate
	{
		virtual void networkMetricsChanged(NetworkPerformanceMetrics& sender,
			NetworkType type, std::size_t preferredByterate) = 0;
		
		virtual void networkMetricsUpgraded(NetworkPerformanceMetrics& sender,
			NetworkType type, std::size_t preferredByterate) = 0;
		
		virtual void networkMetricsDowngraded(NetworkPerformanceMetrics& sender,
			NetworkType type, std::size_t preferredByterate) = 0;

		virtual ~NetworkPerformanceMetricsDelegate() = default;
	};
		
	class NetworkPerformanceMetrics
	{
	public:
		explicit NetworkPerformanceMetrics(std::size_t byteRate = 57600);

		virtual ~NetworkPerformanceMetrics();
        
        void start();
		
        void stop();
		
		void setByteRate(std::size_t value);
		
		void updateData(std::size_t data);
		
		void setDelegate(const std::shared_ptr<NetworkPerformanceMetricsDelegate> delegate);
		
	protected:
		virtual void run(const boost::system::error_code& ec);
		
	private:
		bool byteRateShouldDrop(std::size_t cbs) const;
		
		void clear();
		
		void handleNetworkProfiler(std::size_t cbs);

		void notifyListenersIfNeeded(NetworkType newNetworkType, std::size_t newByteRate);
		
		float newByteRateFromPercentage() const;
		
	private:
		std::size_t byteRate_;
		std::size_t oldCbs_;
		std::chrono::milliseconds stableNetCounter_;
		int networkErrorCounter_;
		boost::posix_time::seconds interval_;
		boost::asio::io_service io_service_;
		boost::asio::deadline_timer timer_;
		std::thread thread_;
		std::atomic_bool stopped_;
		std::mutex mutex_;
		NetworkType currentNetwork_;
		std::weak_ptr<NetworkPerformanceMetricsDelegate> delegate_;
		NetworkSpeedIndicator nsi_;
	};
} // end RtmpKit
