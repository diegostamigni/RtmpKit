//
//  multicast_delegate.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 22/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include <type_traits>
#include <algorithm>
#include <functional>
#include <deque>
#include <memory>
#include <mutex>

namespace RtmpKit
{
	template <typename T,  
		typename = std::enable_if<std::is_const<T>::value
			&& std::is_lvalue_reference<T>::value>>
	class MulticastDelegate
	{
		using Container = std::deque<std::weak_ptr<T>>;
		using ContainerIterator = typename Container::iterator;
		
	public:
		MulticastDelegate() = default;

		MulticastDelegate(const MulticastDelegate&) = default;

		MulticastDelegate(MulticastDelegate&&) = default;

		MulticastDelegate& operator=(const MulticastDelegate&) = default;

		MulticastDelegate& operator=(MulticastDelegate&&) = default;

	public:
		void addDelegate(std::shared_ptr<T> delegate)
		{
			std::lock_guard<std::mutex> _(mutex_);
			deque_.push_back(delegate);
		}

		void operator+=(std::shared_ptr<T> delegate)
		{
			addDelegate(delegate);
		}

		void removeDelegate(std::shared_ptr<T> delegate)
		{
			if (deque_.empty())
			{
				return;
			}
			
			std::lock_guard<std::mutex> _(mutex_);
			auto it = std::find_if(deque_.begin(), deque_.end(), [delegate](const auto &v)
			{
				// TODO(diegostamigni): we might need an hash function or an operator== here..
				if (auto spt = v.lock())
				{
					if (spt.get() == delegate.get())
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				
				return true;
			});

			if (it != deque_.end())
			{
				deque_.erase(it);
			}
		}

		void operator-=(std::shared_ptr<T> delegate)
		{
			removeDelegate(delegate);
		}

		void invoke(const std::function<void(std::shared_ptr<T> )> &invokation)
		{
			if (deque_.empty())
			{
				return;
			}
			
			for (auto &item : deque_)
			{
				if (auto spt = item.lock())
				{
					invokation(spt);
				}
			}
		}

		void operator()(const std::function<void(std::shared_ptr<T>)> &invokation)
		{
			invoke(invokation);
		}

	private:
		std::mutex mutex_;
		Container deque_;
	};
}
