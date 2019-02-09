//
//  parser_test.cc
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#include "gtest/gtest.h"

#include "../utils.h"
#include "../../private/reader.h"
#include "../../private/flv/flv_mediaitem.h"

struct FLVMediaItemDelegate : RtmpKit::FLVMediaItemParserDelegate
{
	virtual void amfPacketReady(RtmpKit::FLVMediaAMFTag tag) override
	{
		std::cout << "amfPacketReady" << '\n';
		std::cout << " - media total duration: " << tag.duration() << '\n';
		std::cout << " - file size: " << tag.fileSize() << "k" << '\n';
		std::cout << " - video size: (" << tag.width() << ", " << tag.height() << ")" << '\n';
	}

	virtual void audioPacketReady(RtmpKit::FLVMediaAudioTag tag) override
	{
		std::cout << "audioPacketReady" << '\n';
		std::cout << " - streamId: " << tag.streamID() << '\n';
		std::cout << " - timestamp: " << tag.timestamp() << '\n';
		std::cout << " - payload (size): " << tag.payload().size() << '\n';
	}

	virtual void videoPacketReady(RtmpKit::FLVMediaVideoTag tag) override
	{
		std::cout << "videoPacketReady" << '\n';
		std::cout << " - streamId: " << tag.streamID() << '\n';
		std::cout << " - timestamp: " << tag.timestamp() << '\n';
		std::cout << " - payload (size): " << tag.payload().size() << '\n';
	}

	void parsingCompleted() override
	{
	}
};

TEST(Parser, ParseFile)
{
	auto videoSampleStream = RtmpKit::FileReader{ big_buck_bunny_720p_2mb_flv };
	auto readResult = videoSampleStream.readAll();

	RtmpKit::FLVMediaItem flvItem;
	flvItem.deserialize(readResult.second.cbegin(), readResult.second.cend());

	ASSERT_EQ(flvItem.header().signature(), RtmpKit::supportedFileTypes.at(RtmpKit::FileType::FLV));
}

TEST(Parser, ParseFileWithDelegate)
{
	auto videoSampleStream = RtmpKit::FileReader{ big_buck_bunny_720p_2mb_flv };
	auto readResult = videoSampleStream.readAll();

	RtmpKit::FLVMediaItem flvItem;
	auto delegate = std::make_shared<FLVMediaItemDelegate>();

	flvItem.addDelegate(delegate);
	flvItem.deserialize(readResult.second.cbegin(), readResult.second.cend());

	ASSERT_EQ(flvItem.header().signature(), RtmpKit::supportedFileTypes.at(RtmpKit::FileType::FLV));
}

TEST(Audio, Serialization)
{
	auto value = RtmpKit::v8{ 0xaf };

	auto tag = RtmpKit::FLVMediaAudioTag{};
	tag.setPayload(value);
	tag.deserialize();

	auto deserialized = tag.serialize();

	ASSERT_TRUE(deserialized.size() == 1);
	ASSERT_EQ(deserialized.front(), value.front());
}

TEST(Video, Serialization)
{
	auto value = RtmpKit::v8{ 0x27 };

	auto tag = RtmpKit::FLVMediaVideoTag{};
	tag.setPayload(value);
	tag.deserialize();

	auto deserialized = tag.serialize();

	ASSERT_TRUE(deserialized.size() == 1);
	ASSERT_EQ(deserialized.front(), value.front());
}
