//
//  queue.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 21/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include <thread>
#include <mutex>
#include <functional>
#include <type_traits>
#include <atomic>
#include <deque>

namespace RtmpKit
{
	template<typename T,
		typename = std::enable_if<std::is_move_constructible<T>::value
			&& std::is_move_assignable<T>::value>>
	class Queue
	{
	public:
		Queue()
			: stopped_(false)
		{
		}

		explicit Queue(const std::function<void(T&&)>& action)
			: stopped_(true)
			, handler_(action)
		{
		}

		virtual ~Queue()
		{
			stop();
		}
		
		void start()
		{
			if (!stopped_) return;
			stopped_ = false;
			
			thread_ = std::thread([this]()
			{
				while(!stopped_)
				{
					run();
				}
			});
		}
		
		void stop()
		{
			if (stopped_) return;
			stopped_ = (true);
			cv_.notify_one();
			
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
		}
		
		bool empty() const
		{
			return queue_.empty();
		}
		
		void clear()
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				queue_.clear();
			}
		}
		
		std::size_t size()
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				return queue_.size();
			}
		}
		
		const T& front()
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				return queue_.front();
			}
		}
		
		const T& back()
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
		
		void removeWithPredicate(const std::function<bool(const T&)>& predicate)
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				while(!empty())
				{
					const auto &item = queue_.front();
					if (!predicate(item))
					{
						break;
					}
					else
					{
						queue_.pop_front();
					}
				}
			}
		}

	protected:
		virtual void run()
		{
			std::unique_lock<std::mutex> lk(mutex_);
			{
				cv_.wait(lk, [this](){ return stopped_ || !queue_.empty(); });
				if (queue_.empty() || stopped_)
				{
					lk.unlock();
					return;
				}
			} lk.unlock();
			
			handler_(std::move(pop()));
		}
		
		void pushBack(T&& item)
		{
			if (stopped_) return;
			
			std::lock_guard<std::mutex> _(mutex_);
			{
				queue_.push_back(std::forward<T>(item));
				cv_.notify_one();
			}
		}
		
		void pushBack(T&& item, const std::function<bool(T)> &predicate)
		{
			std::unique_lock<std::mutex> lg(mutex_);
			const auto canPush = queue_.empty() ? true : predicate(queue_.front());
			
			if (canPush)
			{
				lg.unlock();
				pushBack(std::forward<T>(item));
			}
		}
		
		void pushFront(T&& item)
		{
			if (stopped_) return;
			
			std::lock_guard<std::mutex> _(mutex_);
			{
				queue_.push_front(std::forward<T>(item));
				cv_.notify_one();
			}
		}
		
		void pushFront(T&& item, const std::function<bool(T)> &predicate)
		{
			std::unique_lock<std::mutex> lg(mutex_);
			const auto canPush = queue_.empty() ? true : predicate(queue_.back());
			
			if (canPush)
			{
				lg.unlock();
				pushFront(std::forward<T>(item));
			}
		}
		
		T popFront()
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				if (queue_.empty()) return T();
				auto item = std::move(queue_.front());
				queue_.pop_front();
				return item;
			}
		}
		
		T popBack()
		{
			std::lock_guard<std::mutex> _(mutex_);
			{
				if (queue_.empty()) return T();
				auto item = std::move(queue_.back());
				queue_.pop_back();
				return item;
			}
		}
		
		std::atomic_bool stopped_;
		std::thread thread_;
		std::mutex mutex_;
		std::function<void(T&&)> handler_;
		std::condition_variable cv_;
		std::deque<T> queue_;
	};
}
