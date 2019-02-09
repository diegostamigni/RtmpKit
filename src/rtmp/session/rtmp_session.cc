#include "../../../private/rtmp/session/rtmp_session.h"
#include "../../../private/rtmp/message/command/rtmp_close_stream_command_message.h"
#include "../../../private/logger.h"

#include <boost/algorithm/string.hpp>

using namespace RtmpKit;

RtmpSession::RtmpSession()
	: socket_(std::make_unique<RtmpSocket>())
	, appName_("")
{
}

const std::string& RtmpSession::address() const
{
	return socket_->address();
}

const std::string& RtmpSession::port() const
{
	return socket_->port();
}

void RtmpSession::addDelegate(std::shared_ptr<RtmpSessionDelegate> delegate)
{
	multicastDelegate_ += delegate;
	socket_->addDelegate(delegate);
}

void RtmpSession::removeDelegate(std::shared_ptr<RtmpSessionDelegate> delegate)
{
	multicastDelegate_ -= delegate;
	socket_->removeDelegate(delegate);
}

void RtmpSession::extractAppNameAndStreamID(const std::string& stream)
{
	auto splittedStreamId = std::vector<std::string>{};
	boost::split(splittedStreamId, stream, boost::is_any_of("/"));
	appName_ = !splittedStreamId.empty() ? splittedStreamId.front() : stream;
	streamName_ = !splittedStreamId.empty() ? splittedStreamId.back() : stream;
}

void RtmpSession::start(const std::string& address, const std::string& port,
						const std::string& appName, const std::string& stream)
{
	LOG_DEBUG(str(
		boost::format("Starting the RTMP session for %1%:%2% (%3%:%4%)") % address % port % appName % stream).c_str());
	
	appName_ = appName;
	streamName_ = stream;
	
	socket_->removeDelegate(shared_from_this());
	socket_->addDelegate(shared_from_this());
	
	socket_->connect(address.c_str(), port.c_str());
}

void RtmpSession::start(const std::string& address, const std::string& port, const std::string& streamAndAppName)
{
	LOG_DEBUG(str(
		boost::format("Starting the RTMP session for %1%:%2% (%3%)") % address % port % streamAndAppName).c_str());
	
	extractAppNameAndStreamID(streamAndAppName);
	
	socket_->removeDelegate(shared_from_this());
	socket_->addDelegate(shared_from_this());
	
	socket_->connect(address.c_str(), port.c_str());
}

void RtmpSession::end()
{
	LOG_DEBUG(str(boost::format("Ending the RTMP session for streamID: %1%/%2%") % appName_ % streamName_).c_str());
	
	socket_->close();
}

u8 RtmpSession::version()
{
	// By default, is what rmpt_utils::RtmpProtocolVersion is
	return RtmpProtocolVersion;
}

void RtmpSession::updateCurrentMetrics(std::size_t currentByteRate)
{
	if (currentByteRate > 0)
	{
		socket_->uploadingNetworkPerformanceMetrics().setByteRate(currentByteRate);
		socket_->downloadingNetworkPerformanceMetrics().setByteRate(currentByteRate);
	}
}

void RtmpSession::handshake()
{
	LOG_DEBUG("Starting handshake..");
}

void RtmpSession::handshakeCompleted()
{
}

void RtmpSession::sendCloseStream()
{
	if (!socket_->isOpen())
	{
		RtmpSession::end();
		return;
	}
	
	LOG_DEBUG("Sending the closeStream message");
	
	auto command = RtmpPacketGenerator::newPacket<RtmpCloseStreamCommandMessage>();
	socket_->sendPacket(command, [this](const auto& ec, auto bytes_transferred)
	{
		if (!ec)
		{
			LOG_DEBUG("closeStream message sent");
		}
		else
		{
			LOG_ERROR(str(boost::format("Unable to send the closeStream message: %1%") % ec.message()).c_str());
		}

		this->RtmpSession::end();
	});
}

void RtmpSession::reset()
{
}

void RtmpSession::sessionReady()
{
}

// RtmpSocketDelegate
void RtmpSession::socketWillConnect()
{
	auto &unpm = socket_->uploadingNetworkPerformanceMetrics();
	unpm.setDelegate(shared_from_this());
	
	auto &dnpm = socket_->downloadingNetworkPerformanceMetrics();
	dnpm.setDelegate(shared_from_this());
}

void RtmpSession::socketDidConnect()
{
	handshake();
}

void RtmpSession::socketWillClose()
{
}

void RtmpSession::socketDidClose()
{
}

void RtmpSession::socketFailedToConnect(const std::string& errMessage)
{
}

void RtmpSession::reconnectionRequired()
{
	LOG_INFO("Socket reconnection required");
	multicastDelegate_([](auto d) { d->reconnectionRequired(); });
}

// NetworkPerformanceMetricsDelegate
void RtmpSession::networkMetricsChanged(NetworkPerformanceMetrics& sender,
	NetworkType type, std::size_t preferredByterate)
{
}

void RtmpSession::networkMetricsUpgraded(NetworkPerformanceMetrics& sender,
	NetworkType type, std::size_t preferredByterate)
{
}

void RtmpSession::networkMetricsDowngraded(NetworkPerformanceMetrics& sender,
	NetworkType type, std::size_t preferredByterate)
{
}
