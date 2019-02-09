//
//  rtmp_watcher_session.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 24/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/session/rtmp_client_session.h"
#include "../../../private/rtmp/session/rtmp_session_settings.h"

namespace RtmpKit
{
	struct RtmpWatcherSessionDelegate : public RtmpClientSessionDelegate
	{
		virtual void metadataPacketReceived(RtmpMetadataMessage&& packet) = 0;

		virtual void videoPacketReceived(RtmpVideoMessage&& packet) = 0;

		virtual void audioPacketReceived(RtmpAudioMessage&& packet) = 0;
		
		virtual void networkPerformanceMetricsChanged(NetworkType type, std::size_t preferredByterate) = 0;
	};

	class RtmpWatcherSession : public RtmpClientSession
	{
	public:
		RtmpWatcherSession();

		explicit RtmpWatcherSession(const RtmpSessionSettings& settings);

		virtual void addWatcherDelegate(std::shared_ptr<RtmpWatcherSessionDelegate> delegate);

		virtual void removeWatcherDelegate(std::shared_ptr<RtmpWatcherSessionDelegate> delegate);

		virtual void end() override;
		
		virtual void reset() override;

	protected:
		virtual void handshakeCompleted() override;

		virtual void sendSetChunkSize();

		virtual void sendConnectCommandMessage();

		virtual void receiveWindowAcknowledgeSize();

		virtual void receiveSetPeerBandWidth();

		virtual void receiveSetChunkSize();

		virtual void receiveConnectionCommandResponse();

		virtual void sendWindowAcknowledgeSize();

		virtual void sendCreateStreamCommand();

		virtual void receiveCreateStreamCommandResponse();

		virtual void sessionReady() override;

		virtual void sendPlayCommandMessage();

		virtual void receivePlayOnStatusCommandMessageResponse();

		virtual void receiveRtmpSampleAccessMessage();

		virtual void startMediaPacketRetriever(); 
		
		virtual void receiveStreamBeginMessage();
		
	protected:
		virtual void networkMetricsChanged(NetworkPerformanceMetrics& sender,
			NetworkType type, std::size_t preferredByterate) override;

	private:
		u32 windowAcknowledgementSize_ = 0;
		RtmpSessionSettings settings_;
		bool stopped_ = false;
		std::thread worker_;
		std::weak_ptr<RtmpWatcherSessionDelegate> delegate_;
	};
} // end RtmpKit
