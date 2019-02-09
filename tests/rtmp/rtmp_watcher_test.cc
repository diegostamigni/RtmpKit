//
//  rtmp_watcher_test.cc
//  Diego Stamigni
//
//  Created by Diego Stamigni on 24/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#include "gtest/gtest.h"

#include "../utils.h"
#include "../../private/logger.h"
#include "../../private/semaphore.h"
#include "../../private/reader.h"
#include "../../private/flv/flv_mediaitem.h"
#include "../../private/rtmp/session/rtmp_watcher_session.h"
#include "../../private/flv/flv_amf_mediatag.h"
#include "../../private/flv/flv_audio_mediatag.h"
#include "../../private/flv/flv_video_mediatag.h"

using MediaVariant = RtmpKit::Variant<RtmpKit::FLVMediaAMFTag, RtmpKit::FLVMediaAudioTag, RtmpKit::FLVMediaVideoTag>;
static RtmpKit::SimpleSemaphore sem;

std::shared_ptr<RtmpKit::RtmpWatcherSession> rtmpSession;

struct RtmpWatcherDelegateCollector : RtmpKit::RtmpWatcherSessionDelegate
{
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
	}
	
	void metadataPacketReceived(RtmpKit::RtmpMetadataMessage&& packet) override
	{
		LOG_INFO("metadataPacketReceived()");
	}

	void videoPacketReceived(RtmpKit::RtmpVideoMessage&& packet) override
	{
		LOG_INFO("videoPacketReceived()");
	}

	void audioPacketReceived(RtmpKit::RtmpAudioMessage&& packet) override
	{
		LOG_INFO("audioPacketReceived()");
	}
	
	void networkPerformanceMetricsChanged(RtmpKit::NetworkType type, std::size_t preferredByterate) override
	{
		LOG_INFO("networkPerformanceMetricsChanged()");
	}
};

TEST(RtmpWatcher, CreateSession)
{
	auto delegate = std::make_shared<RtmpWatcherDelegateCollector>();
	rtmpSession = std::make_shared<RtmpKit::RtmpWatcherSession>();

	rtmpSession->addDelegate(delegate);
	rtmpSession->start("stream-stg.diegostamigni.com", 
		"1935", 
		"live", 
		"50c9577b-d484-42b3-9eaa-c962e0398486?RtmpKitChannelId=f2ac27e5-6e27-4901-8614-52b7c71a68ef&RtmpKitClientId=45C78AD6-A3DC-460A-8FCB-AE37AD898538");

	sem.wait();

	rtmpSession->end();
}
