//
//  logger.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 27/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include <cstring>
#include <boost/format.hpp>

#ifdef __APPLE__
	#include <TargetConditionals.h>
#endif

#ifdef ANDROID
	#include <android/log.h>
	#define TAG "RtmpKit"
	#define TARGET_OS_ANDROID 1
#endif

#if defined _WIN32
	#ifndef __cplusplus
		#define __cplusplus
	#endif

	#define __FILENAME__ \
		(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
	#define __FILENAME__ \
		(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#if defined __cplusplus 
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
	#include <iostream>
	#include <sstream>
	#include <string>
	#include <boost/date_time/gregorian_calendar.hpp>
	#include <boost/date_time/posix_time/posix_time.hpp>

	static std::string __current_local_time_and_date__()
	{
		const auto date = boost::gregorian::date(boost::gregorian::day_clock::local_day());
		const auto now = boost::posix_time::microsec_clock::local_time();
		const auto time_of_day = now.time_of_day();
		std::stringstream ss;
		ss
			// date
			<< date.year()
			<< "-" << date.month().as_number()
			<< "-" << date.day().as_number()
			// --
			<< " "
			// time
			<< time_of_day.hours()
			<< ":" << time_of_day.minutes()
			<< ":" << time_of_day.seconds()
			<< ":" << time_of_day.total_milliseconds() % 1000
			// --
		;
		return ss.str();
	}

	#define __CURRENT_TIME__(x) __current_local_time_and_date__()

	#define __TIME_AND_FILENAME_AND_LINE__  \
		boost::format("%1% RtmpKit (%2%:%3%) ") \
			% __CURRENT_TIME__(...) \
			% __FILENAME__ \
			% __LINE__

	#define __PREPREND__ __TIME_AND_FILENAME_AND_LINE__

	#define LOG_INFO(x) std::cout << __PREPREND__ << "[INFO] " << x << '\n'

	#define LOG_ERROR(x) std::cerr << __PREPREND__ << "[ERROR] " <<  x << '\n'

	#define LOG_WARNING(x) std::cout << __PREPREND__ << "[WARNING] " <<  x << '\n'

	#define LOG_FATAL(x) std::cerr << __PREPREND__ << "[FATAL] " <<  x << '\n'

	#define LOG_DEBUG(x) std::cout << __PREPREND__ << "[DEBUG] " <<  x << '\n'
#elif TARGET_OS_ANDROID
	#include <boost/log/trivial.hpp>

	#define LOG_WITH_TYPE(x, y) \
			__android_log_print(ANDROID_LOG_VERBOSE, TAG, "%s", y)
#else
	#include <boost/log/trivial.hpp>

	#define __PREPEND__ boost::format("RtmpKit (%1%:%2%) ") % __FILENAME__ % __LINE__

	#define LOG_WITH_TYPE(x, y) BOOST_LOG_TRIVIAL(x) << __PREPEND__ << y
#endif
#endif

#ifndef LOG_INFO
	#define LOG_INFO(x) LOG_WITH_TYPE(info, x)
#endif

#ifndef LOG_ERROR
	#define LOG_ERROR(x) LOG_WITH_TYPE(error, x)
#endif

#ifndef LOG_WARNING
	#define LOG_WARNING(x) LOG_WITH_TYPE(warning, x)
#endif

#ifndef LOG_FATAL
	#define LOG_FATAL(x) LOG_WITH_TYPE(fatal, x)
#endif

#if defined LOG_VERBOSE
	#ifndef LOG_DEBUG
		#define LOG_DEBUG(x) LOG_WITH_TYPE(debug, x)
	#endif
#else
	#undef LOG_DEBUG
	#define LOG_DEBUG(x)
#endif
