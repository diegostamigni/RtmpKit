//
//  buffer_queue.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 03/02/2017.
//  Copyright © 2017 RtmpKit. All rights reserved.
//

#pragma once

#include "../private/logger.h"
#include "../private/bufferable.h"

#include <mutex>
#include <functional>
#include <type_traits>
#include <atomic>
#include <chrono>
#include <list>

namespace RtmpKit
{
	using namespace std::chrono_literals;
	
    template <typename T, 
        typename = std::enable_if<std::is_base_of<T, Bufferable>::value>>
	class BufferQueue
	{
	public:
		explicit BufferQueue(std::chrono::milliseconds minBufferSize = 500ms)
			: totalDuration_(0s)
			, minBufferSize_(minBufferSize)
			, maxBufferSize_(minBufferSize + (minBufferSize / 3))
		{
		}
		
		virtual ~BufferQueue()
		{
			clear();
		}
		
		std::chrono::milliseconds currentBufferSize() const
		{
			return totalDuration_;
		}
		
		std::chrono::milliseconds minimumBufferSize() const
		{
			return minBufferSize_;
		}
		
		void setMinimumBufferSize(std::chrono::milliseconds value)
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				minBufferSize_ = value;
				maxBufferSize_ = value + (value / 3);
			}
		}
		
		std::chrono::milliseconds maximumBufferSize() const
		{
			return maxBufferSize_;
		}
		
		void syncBufferSize()
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				shrinkToFit(maximumBufferSize());
			}
		}
		
		bool empty() const
		{
			return queue_.empty();
		}
		
		void clear()
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				totalDuration_ = std::chrono::milliseconds(0);
				queue_.clear();
			}
		}
		
		std::size_t size() const
		{
			return queue_.size();
		}
		
		const T& front(bool lock = true)
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				return queue_.front();
			}
		}
		
		const T& back(bool lock = true)
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				return queue_.back();
			}
		}
	
		void push(T&& item)
		{
			pushBack(std::forward<T>(item));
		}
		
		void push(T&& item, const std::function<bool(T)> &predicate)
		{
			pushBack(std::forward<T>(item), predicate);
		}
		
		T pop()
		{
			return popFront();
		}
		
		bool isReady()
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				return currentBufferSize() >= minimumBufferSize();
			}
		}
		
	private:
		std::chrono::milliseconds fetchDuration(const T& item)
		{
			return std::chrono::milliseconds(item.duration());
		}
		
		void handleDurationIncrease(std::chrono::milliseconds value)
		{
			totalDuration_ += value;
		}
		
		void handleDurationIncrease(const T& item)
		{
			handleDurationIncrease(fetchDuration(item));
		}
		
		void handleDurationDecrease(std::chrono::milliseconds value)
		{
			totalDuration_ -= value;
			
			if (totalDuration_.count() < 0)
			{
				totalDuration_ = 0ms;
			}
		}
		
		void handleDurationDecrease(const T& item)
		{
			handleDurationDecrease(fetchDuration(item));
		}
		
		void pushBack(T&& item)
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				queue_.push_back(std::forward<T>(item));
				handleDurationIncrease(item);
			}
		}
		
		T popFront()
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				if (empty()) return T();
				auto item = std::move(queue_.front());
				
				queue_.pop_front();
				handleDurationDecrease(item);
				
				return item;
			}
		}
		
		void shrinkToFit(std::chrono::milliseconds newSize)
		{
			const auto cBufferedSize = currentBufferSize();
			const auto qSize = size();
			
			if (empty() || newSize >= cBufferedSize) return;
			if (newSize.count() == 0)
			{
				// clear requested...
				queue_.clear();
				handleDurationDecrease(cBufferedSize);
				LOG_DEBUG("Shrinking the buffer: clear requested..");
				return;
			}
			
			// TODO(diegostamigni): we're assuming that every packet has the same duration
			// we might want to change this later
			const auto averagePacketDuration = std::chrono::milliseconds(cBufferedSize.count() / qSize);
			const auto diff = cBufferedSize - newSize;
			
			// The diff between the current size and the desired size is too small
			if (diff < averagePacketDuration || averagePacketDuration <= 0ms) return;
			const auto itemsToBeRemoved = diff.count() / averagePacketDuration.count();
			
			LOG_DEBUG(str(boost::format("Shrinking the buffer: removing %1% items because the current buffer " \
				"size is %2%ms but is has to be %3%ms..")
					  % itemsToBeRemoved
					  % cBufferedSize.count()
					  % newSize.count()).c_str());

			auto endIt = queue_.begin();
			std::advance(endIt, itemsToBeRemoved);
			
			queue_.erase(queue_.begin(), endIt);
			handleDurationDecrease(diff);
		}
		
	private:
		std::mutex mutex_;
		std::list<T> queue_;
		std::chrono::milliseconds totalDuration_, minBufferSize_, maxBufferSize_;
	};
} // end RtmpKit
