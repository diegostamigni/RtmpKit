//
//  rtmp_broadcaster_test.cc
//  Diego Stamigni
//
//  Created by Diego Stamigni on 30/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#include "gtest/gtest.h"

#include "../utils.h"
#include "../../private/logger.h"
#include "../../private/queue.h"
#include "../../private/semaphore.h"
#include "../../private/reader.h"
#include "../../private/flv/flv_mediaitem.h"
#include "../../private/rtmp/session/rtmp_broadcaster_session.h"
#include "../../private/flv/flv_amf_mediatag.h"
#include "../../private/flv/flv_audio_mediatag.h"
#include "../../private/flv/flv_video_mediatag.h"

using MediaVariant = RtmpKit::Variant<RtmpKit::FLVMediaAMFTag, RtmpKit::FLVMediaAudioTag, RtmpKit::FLVMediaVideoTag>;

static RtmpKit::SimpleSemaphore sem;

static auto videoSampleStream = RtmpKit::FileReader{
	big_buck_bunny_720p_50mb_flv
//	big_buck_bunny_720p_2mb_flv
//	from_app_flv
};

static auto readResult = videoSampleStream.readAll();

std::unique_ptr<RtmpKit::FLVMediaItem> flvItem;
std::shared_ptr<RtmpKit::RtmpBroadcasterSession> rtmpSession;
std::unique_ptr<RtmpKit::Queue<MediaVariant>> frameVideoQueue;
std::unique_ptr<RtmpKit::Queue<MediaVariant>> frameAudioQueue;

struct RtmpBroadcasterDelegateCollector : RtmpKit::RtmpBroadcasterSessionDelegate, RtmpKit::FLVMediaItemParserDelegate
{
	// RTMP delegate

	virtual void socketWillConnect() override
	{
	}

	virtual void socketDidConnect() override
	{
	}

	virtual void socketWillClose() override
	{
	}

	virtual void socketDidClose() override
	{
		sem.notify();
		ASSERT_TRUE(true);
	}

	virtual void socketFailedToConnect(const std::string& errMessage) override
	{
		sem.notify();
		ASSERT_TRUE(true);
	}
	
	void reconnectionRequired() override
	{
		sem.notify();
		ASSERT_TRUE(true);
	}

	virtual void handshakeDone() override
	{
	}

	virtual void sessionStarted() override
	{
		LOG_INFO("sessionStarted()");

		if (flvItem.get() != nullptr)
		{
			videoSampleStream.seek(0);
			flvItem->deserialize(readResult.second.cbegin(), readResult.second.cend());
		}
	}
	
	void networkPerformanceMetricsChanged(RtmpKit::NetworkType type, std::size_t preferredByterate) override
	{
		LOG_INFO("networkPerformanceMetricsChanged()");
	}

	// FLV delegate

	virtual void amfPacketReady(RtmpKit::FLVMediaAMFTag tag) override
	{
		rtmpSession->sendOnMetaDataMessage(std::move(tag));
	}

	virtual void audioPacketReady(RtmpKit::FLVMediaAudioTag tag) override
	{
		frameAudioQueue->push(std::move(tag));
	}

	virtual void videoPacketReady(RtmpKit::FLVMediaVideoTag tag) override
	{
		frameVideoQueue->push(std::move(tag));
	}

	virtual void parsingCompleted() override
	{
		sem.notify();
	}
};

TEST(RtmpBroadcaster, CreateSessionAndPushFLV)
{
	auto delegate = std::make_shared<RtmpBroadcasterDelegateCollector>();
	rtmpSession = std::make_shared<RtmpKit::RtmpBroadcasterSession>();
	flvItem = std::make_unique<RtmpKit::FLVMediaItem>();

	frameVideoQueue = std::make_unique<RtmpKit::Queue<MediaVariant>>([rs = rtmpSession, this](auto&& item)
	{
		if (auto *tag = boost::get<RtmpKit::FLVMediaVideoTag>(&item))
		{
			rs->sendVideoMessage(std::move(*tag));
		}
	});

	frameAudioQueue = std::make_unique<RtmpKit::Queue<MediaVariant>>([rs = rtmpSession, this](auto&& item)
	{
		if (auto *tag = boost::get<RtmpKit::FLVMediaAudioTag>(&item))
		{
			rs->sendAudioMessage(std::move(*tag));
		}
	});

	rtmpSession->addBroadcasterDelegate(delegate);
	flvItem->addDelegate(delegate);
	rtmpSession->start("stream-stg.diegostamigni.com", "1935", "live/1f34ae44-034b-4420-9b59-2fabd39e8ab2?RtmpKitChannelId=f2ac27e5-6e27-4901-8614-52b7c71a68ef&RtmpKitClientId=8EE512BE-AEDA-4D51-85B1-5176E8532AD1");

	sem.wait();

	rtmpSession->end();
}
