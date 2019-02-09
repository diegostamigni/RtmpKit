//
//  rtmp_broadcaster_session.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 30/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/flv/flv_amf_mediatag.h"
#include "../../../private/flv/flv_audio_mediatag.h"
#include "../../../private/flv/flv_video_mediatag.h"
#include "../../../private/rtmp/session/rtmp_client_session.h"
#include "../../../private/rtmp/session/rtmp_session_settings.h"

namespace RtmpKit
{
	struct RtmpBroadcasterSessionDelegate : public RtmpClientSessionDelegate
	{
		virtual void networkPerformanceMetricsChanged(NetworkType type, std::size_t preferredByterate) = 0;
	};
	
	class RtmpBroadcasterSession : public RtmpClientSession
	{
	public:
		RtmpBroadcasterSession();

		explicit RtmpBroadcasterSession(const RtmpSessionSettings& settings);

		void sendOnMetaDataMessage();

		void sendOnMetaDataMessage(FLVMediaAMFTag&& mediaTag);

		void sendAudioMessage(u32 timestamp, v8&& data, bool isParam = false);

		void sendAudioMessage(FLVMediaAudioTag&& tag);

		void sendVideoMessage(FLVVideoFrameType frametype, u32 timestamp, v8&& data, bool isParam = false);

		void sendVideoMessage(FLVMediaVideoTag&& tag);

		virtual void end() override;
		
		virtual void reset() override;
		
		void addBroadcasterDelegate(std::shared_ptr<RtmpBroadcasterSessionDelegate> delegate);
		
		void removeBroadcasterDelegate(std::shared_ptr<RtmpBroadcasterSessionDelegate> delegate);

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

		virtual void sendPublishCommand();

		virtual void receiveOnStatusCommandResponse();

		virtual void sessionReady() override;
		
	protected:
		virtual void networkMetricsChanged(NetworkPerformanceMetrics& sender,
			NetworkType type, std::size_t preferredByterate) override;

	private:
		u32 windowAcknowledgementSize_ = 0;
		RtmpSessionSettings settings_;
		std::weak_ptr<RtmpBroadcasterSessionDelegate> delegate_;
	};
} // end RtmpKit
