#include "../../../private/rtmp/session/rtmp_broadcaster_session.h"
#include "../../../private/rtmp/message/command/rtmp_connect_command_response_message.h"
#include "../../../private/rtmp/message/command/rtmp_create_stream_command_response_message.h"
#include "../../../private/rtmp/message/command/rtmp_publish_command_message.h"
#include "../../../private/rtmp/message/command/rtmp_onstatus_command_response_message.h"
#include "../../../private/rtmp/message/control/rtmp_window_ack_size_message.h"
#include "../../../private/rtmp/message/control/rtmp_set_peer_bandwidth_message.h"
#include "../../../private/rtmp/message/control/rtmp_set_chunk_size_message.h"
#include "../../../private/logger.h"

using namespace RtmpKit;

RtmpBroadcasterSession::RtmpBroadcasterSession()
{
}

RtmpBroadcasterSession::RtmpBroadcasterSession(const RtmpSessionSettings& settings)
	: settings_{ settings }
{
}

void RtmpBroadcasterSession::addBroadcasterDelegate(std::shared_ptr<RtmpBroadcasterSessionDelegate> delegate)
{
	RtmpClientSession::addDelegate(delegate);
	delegate_ = delegate;
}

void RtmpBroadcasterSession::removeBroadcasterDelegate(std::shared_ptr<RtmpBroadcasterSessionDelegate> delegate)
{
	RtmpClientSession::removeDelegate(delegate);
	delegate_.reset();
}

void RtmpBroadcasterSession::handshakeCompleted()
{
	RtmpClientSession::handshakeCompleted();

	sendSetChunkSize();
}

void RtmpBroadcasterSession::sendSetChunkSize()
{
	if (!socket_->isOpen()) return;

	LOG_DEBUG(str(boost::format("Sending set chunk size command message: %1%") 
		% settings_.chunkSize()).c_str());

	auto command = RtmpPacketGenerator::newPacket<RtmpSetChunkSizeMessage>(settings_.chunkSize());

	socket_->sendPacket(command, [this] (const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("set chunk size command message sent");
			this->sendConnectCommandMessage();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send the set chunk size message: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::sendConnectCommandMessage()
{
	if (!socket_->isOpen()) return;

	LOG_DEBUG("Sending connect command message");
	auto command = RtmpPacketGenerator::newPacket<RtmpConnectCommandMessage>(address(), port(), appName());

	socket_->sendPacket(command, [this] (const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("Connect command message sent");
			this->receiveWindowAcknowledgeSize();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send the command message: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::receiveWindowAcknowledgeSize()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Receiving the Window Acknowledge Size");

	socket_->receivePacket<RtmpWindowAcknowledgementSizeMessage>([this](const auto& ec, auto size, auto &&packet)
	{
		if (!ec)
		{
			this->windowAcknowledgementSize_ = packet.windowAcknowledgementSize();
			LOG_DEBUG(str(boost::format("Window Acknowledge Size received: %1%") 
				% this->windowAcknowledgementSize_).c_str());
			this->receiveSetPeerBandWidth();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the Window Acknowledge Size message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::receiveSetPeerBandWidth()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Receiving the Set Peer Bandwidth");

	socket_->receivePacket<RtmpSetPeerBandWidthMessage>([this](const auto& ec, auto size, auto &&packet)
	{
		if (!ec)
		{
			LOG_DEBUG("Set Peer Bandwidth received");
			this->sendWindowAcknowledgeSize();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the Set Peer Bandwidth message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::sendWindowAcknowledgeSize()
{
	if (!socket_->isOpen()) return;

	if (windowAcknowledgementSize_ <= 0)
	{
		LOG_FATAL("Window Acknowledge Size is wrong or not received");
		return;
	}

	LOG_DEBUG("Sending Window Acknowledge Size message");
	auto command = RtmpPacketGenerator::newPacket<RtmpWindowAcknowledgementSizeMessage>(windowAcknowledgementSize_);

	socket_->sendPacket(command, [this](const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("Window Acknowledge Size message sent");
			this->receiveSetChunkSize();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send the Window Acknowledge Size message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::receiveSetChunkSize()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Receiving the Set Chunk Size");

	socket_->receivePacket<RtmpSetChunkSizeMessage>([this](const auto& ec, auto size, auto &&packet)
	{
		if (!ec)
		{
			LOG_DEBUG(str(boost::format("Set Chunk Size received: %1%") % packet.chunkSize()).c_str());
			this->settings_.setChunkSize(packet.chunkSize());
			this->receiveConnectionCommandResponse();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the Set Chunk Size message: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::receiveConnectionCommandResponse()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Receiving the connect command response");

	socket_->receiveDynamicPacket<RtmpConnectCommandResponseMessage>([this](const auto& ec, auto size, auto &&packet)
	{
		if (!ec)
		{
			LOG_DEBUG("Connect command response received");
			LOG_INFO(str(boost::format("Connect command response: %1%") % packet.description()).c_str());
			
			if (packet.isSuccessfull())
			{
				this->sendCreateStreamCommand();
			}
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the connect command response message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::sendCreateStreamCommand()
{
	if (!socket_->isOpen()) return;

	LOG_DEBUG("Sending the create stream command");
	auto command = RtmpPacketGenerator::newPacket<RtmpCreateStreamCommandMessage>();

	socket_->sendPacket(command, [this](const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("Create stream command message sent");
			this->receiveCreateStreamCommandResponse();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send the create stream command message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::receiveCreateStreamCommandResponse()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Receiving the create stream command response");

	socket_->receiveDynamicPacket<RtmpCreateStreamCommandResponseMessage>([this](const auto& ec, auto size, auto &&packet)
	{
		if (!ec)
		{
			LOG_DEBUG("Create Stream command response received");
			LOG_INFO(str(boost::format("Create Stream command response: %1%") % packet.description()).c_str());

			if (packet.isSuccessfull())
			{
				this->sendPublishCommand();
			}
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the create stream command response message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::sendPublishCommand()
{
	if (!socket_->isOpen()) return;

	LOG_DEBUG(str(boost::format("Sending the publish command for the streamID: %1%") % streamID()).c_str());
	auto command = RtmpPacketGenerator::newPacket<RtmpPublishCommandMessage>(streamID());

	socket_->sendPacket(command, [this](const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("Publish command message sent");
			this->receiveOnStatusCommandResponse();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send the publish command message: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::receiveOnStatusCommandResponse()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Receiving the OnStatus command response");

	socket_->receiveDynamicPacket<RtmpOnStatusCommandResponseMessage>([this](const auto& ec, auto size, auto &&packet)
	{
		if (!ec)
		{
			LOG_DEBUG("OnStatus response received");
			LOG_INFO(str(boost::format("OnStatus response: %1%") % packet.description()).c_str());

			if (packet.isSuccessfull())
			{
				this->sessionReady();
			}
			else
			{
				this->multicastDelegate_([](auto d) { d->reconnectionRequired(); });
				this->socket_->close();
			}
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the OnStatus command response message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::sessionReady()
{
	RtmpSession::sessionReady();
	if (!socket_->isOpen()) return;
	
	auto& npm = socket_->uploadingNetworkPerformanceMetrics();
	npm.start();
	
	LOG_DEBUG("The session is now ready to use");
	multicastDelegate_([](auto d) { d->sessionStarted(); });
}

void RtmpBroadcasterSession::sendOnMetaDataMessage()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Sending the OnMetaData message");

	auto tag = FLVMediaAMFTag{};
	tag.setWidth(settings_.width());
	tag.setWidth(settings_.height());
	tag.setAudioChannels(settings_.audioChannels());
	tag.setFrameRate(settings_.frameRate());
	tag.setVideoDataRate(settings_.videoDataRate());
	tag.setVideoFrameRate(settings_.videoFrameRate());

	auto command = RtmpPacketGenerator::newPacket<RtmpMetadataMessage>(std::forward<FLVMediaAMFTag>(tag));
	updateCurrentMetrics(static_cast<std::size_t>(std::ceil((settings_.videoDataRate() * 1024) / 8)));
	
	socket_->sendPacket(command, [this](const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("OnMetaData message sent");
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send the OnMetaData message: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::sendOnMetaDataMessage(FLVMediaAMFTag&& mediaTag)
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Sending the OnMetaData message");
	
	auto command = RtmpPacketGenerator::newPacket<RtmpMetadataMessage>(std::forward<FLVMediaAMFTag>(mediaTag));
	auto currentByteRate = static_cast<std::size_t>(std::ceil((command.videoDataRate() * 1024) / 8));
	updateCurrentMetrics(currentByteRate);
	
	socket_->sendPacket(command, [this](const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("OnMetaData message sent");
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send the OnMetaData message: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpBroadcasterSession::sendAudioMessage(u32 timestamp, v8&& data, bool isParam)
{
	if (!socket_->isOpen()) return;
	
	auto mediaTag = FLVMediaAudioTag{};
	mediaTag.setTimestamp(timestamp);
	mediaTag.setSoundFormat(settings_.soundFormat());
	mediaTag.setSoundRate(settings_.soundRate());
	mediaTag.setSoundSampleSize(settings_.soundSampleSize());
	mediaTag.setSoundType(settings_.soundType());
	mediaTag.setPayload(std::forward<v8>(data));
	
	if (!isParam)
	{
		mediaTag.updateMedataData();
	}
	else
	{
		mediaTag.setPayloadSize(static_cast<u24>(mediaTag.payload().size()));
	}
	
	auto packets = RtmpPacketGenerator::newChunkedPacket<RtmpAudioMessage>(settings_.chunkSize(), std::move(mediaTag));
	
	for (auto&& packet : packets)
	{
		if (!socket_->isOpen()) break;

		socket_->sendPacket(packet, [this](const auto& ec, auto bytes_transferred)
		{
			if (!ec)
			{
				// ?
			}
			else
			{
				LOG_ERROR(str(boost::format("Unable to send this audio message: %1%") % ec.message()).c_str());
				socket_->close();
			}
		});
	}
}

void RtmpBroadcasterSession::sendAudioMessage(FLVMediaAudioTag&& mediaTag)
{
	if (!socket_->isOpen()) return;

	auto packets = RtmpPacketGenerator::newChunkedPacket<RtmpAudioMessage>(settings_.chunkSize(),
		std::forward<FLVMediaAudioTag>(mediaTag));

	for (auto&& packet : packets)
	{
		if (!socket_->isOpen()) break;

		socket_->sendPacket(packet, [this](const auto& ec, auto bytes_transferred)
		{
			if (!ec)
			{
				// ?
			}
			else
			{
				LOG_ERROR(str(boost::format("Unable to send this audio message: %1%") % ec.message()).c_str());
				socket_->close();
			}
		});
	}
}

void RtmpBroadcasterSession::sendVideoMessage(FLVVideoFrameType frameType, u32 timestamp, v8&& data, bool isParam)
{
	if (!socket_->isOpen()) return;
	
	auto mediaTag = FLVMediaVideoTag{};
	mediaTag.setTimestamp(timestamp);
	mediaTag.setCodecId(settings_.videoCodecId());
	mediaTag.setFrameType(frameType);
	mediaTag.setPayload(std::forward<v8>(data));
	
	if (!isParam)
	{
		mediaTag.updateMedataData();
	}
	else
	{
		mediaTag.setPayloadSize(static_cast<u24>(mediaTag.payload().size()));
	}
	
	auto packets = RtmpPacketGenerator::newChunkedPacket<RtmpVideoMessage>(settings_.chunkSize(), std::move(mediaTag));
	
	for (auto&& packet : packets)
	{
		if (!socket_->isOpen()) break;
		
		socket_->sendPacket(packet, [this](const auto& ec, auto bytes_transferred)
		{
			if (!ec)
			{
				// ?
			}
			else
			{
				LOG_ERROR(str(boost::format("Unable to send this video message: %1%") % ec.message()).c_str());
				this->socket_->close();
			}
		});
	}
}

void RtmpBroadcasterSession::sendVideoMessage(FLVMediaVideoTag&& mediaTag)
{
	if (!socket_->isOpen()) return;

	auto packets = RtmpPacketGenerator::newChunkedPacket<RtmpVideoMessage>(settings_.chunkSize(),
		std::forward<FLVMediaVideoTag>(mediaTag));

	for (auto&& packet : packets)
	{
		if (!socket_->isOpen()) break;

		socket_->sendPacket(packet, [this](const auto& ec, auto bytes_transferred)
		{
			if (!ec)
			{
				// ?
			}
			else
			{
				LOG_ERROR(str(boost::format("Unable to send this video message: %1%") % ec.message()).c_str());
				this->socket_->close();
			}
		});
	}
}

void RtmpBroadcasterSession::end()
{
	sendCloseStream();
}

void RtmpBroadcasterSession::reset()
{
	RtmpSession::reset();
}

void RtmpBroadcasterSession::networkMetricsChanged(NetworkPerformanceMetrics& sender,
	NetworkType type, std::size_t preferredByterate)
{
	using namespace std::chrono_literals;
	auto &dnpm = socket_->downloadingNetworkPerformanceMetrics();
	
	if (&sender == &dnpm)
	{
		// we're not interested in the download speed metrics here
		return;
	}
	
	if (auto ptr = delegate_.lock())
	{
		ptr->networkPerformanceMetricsChanged(type, preferredByterate);
	}
}
