//
//  utils.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 12/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <boost/variant.hpp>

namespace RtmpKit
{
	template <typename... T>
	using Variant = boost::variant<T...>;
	
	using u8 = unsigned char;
	using v8 = ::std::vector<u8>;
	using u16 = uint16_t;
	using u24 = uint32_t;
	using u32 = uint32_t;
	using u64 = uint64_t;

	template <typename T>
	using isVector = std::is_same<T, std::vector<typename T::value_type, typename T::allocator_type>>;

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static v8 fromBigEndian64(const T& data)
	{
		auto result = v8(8);
		::std::reverse_copy(data.cbegin(), data.cend(), ::std::begin(result));
		return result;
	}

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static v8 fromLittleEndian64(const T& data)
	{
		auto result = v8(8);
		::std::copy(data.cbegin(), data.cend(), ::std::begin(result));
		return result;
	}

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static u32 fromBigEndian32(const T& data)
	{
		if (data.empty()) return 0;
		return static_cast<u32>((data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24));
	}

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static u32 fromLittleEndian32(const T& data)
	{
		if (data.empty()) return 0;
		return static_cast<u32>((data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
	}

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static u24 fromBigEndian24(const T& data)
	{
		if (data.empty()) return 0;
		return static_cast<u32>((data[2] << 0) | (data[1] << 8) | (data[0] << 16));
	}

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static u24 fromLittleEndian24(const T& data)
	{
		if (data.empty()) return 0;
		return static_cast<u32>((data[0] << 0) | (data[1] << 8) | (data[2] << 16));
	}

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static u16 fromBigEndian16(const T& data)
	{
		if (data.empty()) return 0;
		return static_cast<u32>((data[1] << 0) | (data[0] << 8));
	}

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static u16 fromLittleEndian16(const T& data)
	{
		if (data.empty()) return 0;
		return static_cast<u32>((data[0] << 0) | (data[1] << 8));
	}

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static RtmpKit::v8 toBigEndian64(const T& data)
	{
		auto result = v8(data.size(), 0);
		::std::reverse_copy(data.cbegin(), data.cend(), result.begin());
		return result;
	}

	template <typename T = RtmpKit::v8, typename = std::enable_if<RtmpKit::isVector<T>::value>>
	static RtmpKit::v8 toLittleEndian64(const T& data)
	{
		auto result = v8(data.size(), 0);
		::std::copy(data.cbegin(), data.cend(), result.begin());
		return result;
	}

	static RtmpKit::v8 toLittleEndian32(u32 data)
	{
		return v8{
			static_cast<u8>(data),
			static_cast<u8>(((data >> 8) & 0xFF)),
			static_cast<u8>(((data >> 16) & 0xFF)),
			static_cast<u8>(((data >> 24) & 0xFF))
		};
	}

	static RtmpKit::v8 toBigEndian32(u32 data)
	{
		return v8{
			static_cast<u8>(((data >> 24) & 0xFF)),
			static_cast<u8>(((data >> 16) & 0xFF)),
			static_cast<u8>(((data >> 8) & 0xFF)),
			static_cast<u8>(data),
		};
	}

	static RtmpKit::v8 toLittleEndian24(u24 data)
	{
		return v8{
			static_cast<u8>(data),
			static_cast<u8>(((data >> 8) & 0xFF)),
			static_cast<u8>(((data >> 16) & 0xFF)),
		};
	}

	static RtmpKit::v8 toBigEndian24(u24 data)
	{
		return v8{
			static_cast<u8>(((data >> 16) & 0xFF)),
			static_cast<u8>(((data >> 8) & 0xFF)),
			static_cast<u8>(data),
		};
	}

	static RtmpKit::v8 toLittleEndian16(u16 data)
	{
		return v8{
			static_cast<u8>(data),
			static_cast<u8>(((data >> 8) & 0xFF)),
		};
	}

	static RtmpKit::v8 toBigEndian16(u16 data)
	{
		return v8{
			static_cast<u8>(((data >> 8) & 0xFF)),
			static_cast<u8>(data),
		};
	}

	template <typename T>
	static T lexicalCast(const std::string& str)
	{
		T var;
		::std::istringstream iss;

		iss.str(str);
		iss >> var;

		return var;
	}
    
    static int numberOfSetBits(RtmpKit::u32 i)
    {
        i = i - ((i >> 1) & 0x55555555);
        i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
        return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
    }

	class OutOfBoundException : public std::exception
	{
	public:
		explicit OutOfBoundException(char const* what) : what_(what) {}

		virtual const char* what() const noexcept override
		{
			return what_;
		}

	private:
		char const* what_;
	};
} // end RtmpKit
