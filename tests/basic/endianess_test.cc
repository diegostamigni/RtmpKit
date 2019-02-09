//
//  endianess_test.cc
//  Diego Stamigni
//
//  Created by Diego Stamigni on 2/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#include "gtest/gtest.h"
#include "../../private/utils.h"

TEST(Endianess, BigEndian16)
{
	RtmpKit::u16 value = 42;
	auto converted = RtmpKit::toBigEndian16(value);
	auto result = RtmpKit::fromBigEndian16(converted);
	ASSERT_EQ(value, result);
}

TEST(Endianess, BigEndian24)
{
	RtmpKit::u24 value = 42;
	auto converted = RtmpKit::toBigEndian24(value);
	auto result = RtmpKit::fromBigEndian24(converted);
	ASSERT_EQ(value, result);
}

TEST(Endianess, BigEndian32)
{
	RtmpKit::u32 value = 42;
	auto converted = RtmpKit::toBigEndian32(value);
	auto result = RtmpKit::fromBigEndian32(converted);
	ASSERT_EQ(value, result);
}
