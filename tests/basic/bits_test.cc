//
//  bits.cc
//  Diego Stamigni
//
//  Created by Diego Stamigni on 17/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#include "gtest/gtest.h"

#include "../utils.h"
#include "../../private/utils.h"

TEST(Bits, BeLe)
{
	auto bytes = RtmpKit::v8 {0x00, 0x00, 0xe0};
	auto value = RtmpKit::fromBigEndian24(bytes);

	ASSERT_EQ(value, static_cast<const unsigned int>(224));
}
