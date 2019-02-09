#include "../../../private/rtmp/session/rtmp_watcher_session.h"
#include "../../../private/rtmp/message/command/rtmp_connect_command_response_message.h"
#include "../../../private/rtmp/message/command/rtmp_create_stream_command_response_message.h"
#include "../../../private/rtmp/message/command/rtmp_play_command_message.h"
#include "../../../private/rtmp/message/command/rtmp_onstatus_command_response_message.h"
#include "../../../private/rtmp/message/command/rtmp_sample_access_message.h"
#include "../../../private/rtmp/message/control/rtmp_stream_begin_control_message.h"
#include "../../../private/rtmp/message/control/rtmp_window_ack_size_message.h"
#include "../../../private/rtmp/message/control/rtmp_set_peer_bandwidth_message.h"
#include "../../../private/rtmp/message/control/rtmp_set_chunk_size_message.h"
#include "../../../private/logger.h"

using namespace RtmpKit;

RtmpWatcherSession::RtmpWatcherSession()
	: windowAcknowledgementSize_ { 0 }
	, stopped_{ true }
{
}

RtmpWatcherSession::RtmpWatcherSession(const RtmpSessionSettings& settings)
	: windowAcknowledgementSize_ { 0 }
	, settings_{ settings }
	, stopped_{ true }
{
}

void RtmpWatcherSession::addWatcherDelegate(std::shared_ptr<RtmpWatcherSessionDelegate> delegate)
{
	RtmpClientSession::addDelegate(delegate);
	delegate_ = delegate;
}

void RtmpWatcherSession::removeWatcherDelegate(std::shared_ptr<RtmpWatcherSessionDelegate> delegate)
{
	RtmpClientSession::removeDelegate(delegate);
	delegate_.reset();
}

void RtmpWatcherSession::handshakeCompleted()
{
	RtmpClientSession::handshakeCompleted();

	sendSetChunkSize();
}

void RtmpWatcherSession::sendSetChunkSize()
{
	if (!socket_->isOpen()) return;

	LOG_DEBUG(str(boost::format("Sending set chunk size command message: %1%") % settings_.chunkSize()).c_str());
	auto command = RtmpPacketGenerator::newPacket<RtmpSetChunkSizeMessage>(settings_.chunkSize());

	socket_->sendPacket(command, [this](const auto& ec, auto bytes_transferred)
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

void RtmpWatcherSession::sendConnectCommandMessage()
{
	if (!socket_->isOpen()) return;

	LOG_DEBUG("Sending connect command message");
	auto command = RtmpPacketGenerator::newPacket<RtmpConnectCommandMessage>(address(), port(), appName());

	socket_->sendPacket(command, [this](const auto& ec, auto bytes_transferred)
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

void RtmpWatcherSession::receiveWindowAcknowledgeSize()
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

void RtmpWatcherSession::receiveSetPeerBandWidth()
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
			LOG_ERROR(
				str(boost::format("Unable to receive the Set Peer Bandwidth message: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpWatcherSession::sendWindowAcknowledgeSize()
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

void RtmpWatcherSession::receiveSetChunkSize()
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

void RtmpWatcherSession::receiveConnectionCommandResponse()
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

void RtmpWatcherSession::sendCreateStreamCommand()
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

void RtmpWatcherSession::receiveCreateStreamCommandResponse()
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
				this->sendPlayCommandMessage();
			}
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the create stream command response message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpWatcherSession::sendPlayCommandMessage()
{
	if (!socket_->isOpen()) return;

	LOG_DEBUG(str(boost::format("Sending the play command for streamName: %1%") % streamID()).c_str());
	auto command = RtmpPacketGenerator::newPacket<RtmpPlayCommandMessage>(streamID());

	socket_->sendPacket(command, [this](const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("Play command message sent");
			this->receiveStreamBeginMessage();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send the play command message: %1%") % ec.message()).c_str());
		}
	});
}

void RtmpWatcherSession::receiveStreamBeginMessage()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Receiving the Stream Begin control message");

	socket_->receiveDynamicPacket<RtmpStreamBeginControlMessage>([this](const auto& ec, auto size, auto &&packet)
	{
		if (!ec)
		{
			LOG_DEBUG("Stream Begin control message received");
			this->receivePlayOnStatusCommandMessageResponse();
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the Stream Begin control message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpWatcherSession::receivePlayOnStatusCommandMessageResponse()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Receiving the play OnStatus command response");

	socket_->receiveDynamicPacket<RtmpOnStatusCommandResponseMessage>([this](const auto& ec, auto size, auto &&packet)
	{
		if (!ec)
		{
			LOG_DEBUG("Play OnStatus command response received");
			LOG_INFO(str(boost::format("Play OnStatus command response: %1%") % packet.description()).c_str());

			if (packet.isSuccessfull())
			{
				this->receiveRtmpSampleAccessMessage();
			}
			else
			{
				this->multicastDelegate_([](auto d) { d->reconnectionRequired(); });
				this->socket_->close();
			}
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the play OnStatus command response message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpWatcherSession::receiveRtmpSampleAccessMessage()
{
	if (!socket_->isOpen()) return;
	LOG_DEBUG("Receiving the RtmpSampleAccess command");

	socket_->receiveDynamicPacket<RtmpSampleAccessCommandMessage>([this](const auto& ec, auto size, auto &&packet)
	{
		if (!ec)
		{
			LOG_DEBUG("RtmpSampleAccess command received");
			LOG_INFO(str(boost::format("RtmpSampleAccess command: isVideoAllowed(%1%), isAudioAllowed(%2%)")
				% packet.isVideoAllowed()
				% packet.isAudioAllowed()).c_str());

			if (packet.isSuccessfull())
			{
				this->sessionReady();
				this->startMediaPacketRetriever();
			}
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to receive the RtmpSampleAccess command message: %1%") 
				% ec.message()).c_str());
		}
	});
}

void RtmpWatcherSession::startMediaPacketRetriever()
{
	LOG_DEBUG("Starting the media retriever routine");
    
    if (!stopped_)
    {
        LOG_DEBUG("The media retriever seems to be already running, close and join first.");
        return;
    }
	
	stopped_ = false;
	
    worker_ = std::thread([maxChunkSize = settings_.chunkSize(), this]()
    {
        while(!stopped_)
        {
			this->socket_->receiveDynamicPacketWithDynamicHeader(maxChunkSize,
                [this](const auto& ec, auto size, auto &&packet)
            {
                if (ec || size <= 0)
				{
					return;
				}
				
				if (auto *p = boost::get<RtmpMetadataMessage>(&packet))
				{
					const auto currentByteRate = std::ceil((p->videoDataRate() * 1024) / 8);
					this->updateCurrentMetrics(static_cast<std::size_t>(currentByteRate));
					
					if (auto ptr = this->delegate_.lock())
					{
						ptr->metadataPacketReceived(std::move(*p));
					}
				}
				
				if (auto *p = boost::get<RtmpVideoMessage>(&packet))
				{
					if (auto ptr = this->delegate_.lock())
					{
						ptr->videoPacketReceived(std::move(*p));
					}
				}
				
				if (auto *p = boost::get<RtmpAudioMessage>(&packet))
				{
					if (auto ptr = this->delegate_.lock())
					{
						ptr->audioPacketReceived(std::move(*p));
					}
				}
			});
        }
    });
}

void RtmpWatcherSession::end()
{
	stopped_ = true;
	
    if (std::this_thread::get_id() != worker_.get_id())
    {
        if (worker_.joinable())
        {
            worker_.join();
        }
    }
    else
    {
        if (worker_.joinable())
        {
            worker_.detach();
        }
    }
	
	RtmpSession::end();
	// sendCloseStream();
}

void RtmpWatcherSession::sessionReady()
{
	RtmpSession::sessionReady();
	if (!socket_->isOpen()) return;
	
	auto& npm = socket_->downloadingNetworkPerformanceMetrics();
	npm.start();
	
	LOG_DEBUG("The session is now ready to use");
	multicastDelegate_([](auto d) { d->sessionStarted(); });
}

void RtmpWatcherSession::networkMetricsChanged(NetworkPerformanceMetrics& sender,
	NetworkType type, std::size_t preferredByterate)
{
	using namespace std::chrono_literals;
	auto &unpm = socket_->uploadingNetworkPerformanceMetrics();
	
	if (&sender == &unpm)
	{
		// we're not interested in the upload speed metrics here
		return;
	}
	
	if (auto ptr = delegate_.lock())
	{
		ptr->networkPerformanceMetricsChanged(type, preferredByterate);
	}
}

void RtmpWatcherSession::reset()
{
	RtmpSession::reset();
}
