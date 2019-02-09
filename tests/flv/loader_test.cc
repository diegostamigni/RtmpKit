//
//  loader_test.cc
//  Diego Stamigni
//
//  Created by Diego Stamigni on 12/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#include "gtest/gtest.h"

#include "../utils.h"
#include "../../private/reader.h"
#include "../../private/constants.h"

TEST(Loader, GetFileSize)
{
	auto videoSampleStream = RtmpKit::FileReader(big_buck_bunny_720p_2mb_flv);
	auto videoSize = videoSampleStream.size();

	ASSERT_EQ(videoSize, 2097492);
}

TEST(Loader, LoadFile)
{
	auto f1 = RtmpKit::FileReader(big_buck_bunny_720p_2mb_flv);
	ASSERT_TRUE(f1.exist());

	auto f2 = RtmpKit::FileReader("");
	ASSERT_FALSE(f2.exist());
}

TEST(Loader, ReadFile)
{
	auto videoSampleStream = RtmpKit::FileReader(big_buck_bunny_720p_2mb_flv);
	auto streamSize = videoSampleStream.size();

	// all
	auto readResult = videoSampleStream.readAll();
	auto bytesRead = readResult.first;

	ASSERT_EQ(bytesRead, streamSize);

	// all bytes
	videoSampleStream.seek(0, std::ios_base::beg);
	readResult = videoSampleStream.read(streamSize);
	bytesRead = readResult.first;

	ASSERT_EQ(bytesRead, streamSize);

	// only `rtmpkit::videoReadBufferSize' bytes
	videoSampleStream.seek(0, std::ios_base::beg);
	readResult = videoSampleStream.read(RtmpKit::videoReadBufferSize);
	bytesRead = readResult.first;

	ASSERT_EQ(bytesRead, RtmpKit::videoReadBufferSize);

	// only `3' bytes
	videoSampleStream.seek(0, std::ios_base::beg);
	readResult = videoSampleStream.read(3);
	bytesRead = readResult.first;

	ASSERT_EQ(bytesRead, 3);
}

TEST(Loader, FileType)
{
	auto videoSampleStream1 = RtmpKit::FileReader(big_buck_bunny_720p_2mb_flv);
	ASSERT_EQ(videoSampleStream1.type(), RtmpKit::FileType::FLV);

	auto videoSampleStream2 = RtmpKit::FileReader(sample_text_file_txt);
	ASSERT_FALSE(videoSampleStream2.type() == RtmpKit::FileType::FLV);

	auto videoSampleStream3 = RtmpKit::FileReader(big_buck_bunny_240p_1mb_mp4);
	ASSERT_FALSE(videoSampleStream3.type() == RtmpKit::FileType::FLV);
}