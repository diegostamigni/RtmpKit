#include "../../private/socket/base_socket.h"
#include "../../private/logger.h"

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

#ifndef ENABLE_ASYNC_SOCKET_READ_WRITE
	#define ENABLE_ASYNC_SOCKET_READ_WRITE true
#endif

using namespace RtmpKit;
static const auto socketExpirationTimeout = boost::posix_time::seconds(5);

template <typename SyncReadStream, typename MutableBufferSequence>
std::size_t readWithTimeout(SyncReadStream& s, const MutableBufferSequence& buffers,
	const boost::asio::deadline_timer::duration_type& expiry_time, std::size_t size, boost::system::error_code& ec)
{
	std::size_t bytesTransferred = 0;
	boost::optional<boost::system::error_code> timer_result, err_result = boost::none;
	boost::asio::deadline_timer timer(s.get_io_service());
	timer.expires_from_now(expiry_time);
	
	timer.async_wait([&timer_result] (const boost::system::error_code& error)
	{
		timer_result.reset(error);
	});
	
	boost::asio::async_read(s, buffers, boost::asio::transfer_exactly(size),
		[&err_result, &bytesTransferred] (const auto& error, auto bytes_transferred)
	{
		bytesTransferred = bytes_transferred;
		err_result.reset(error);
	});
	
	s.get_io_service().reset();
	while (s.get_io_service().run_one())
	{
		if (err_result)
		{
			timer.cancel();
		}
		else if (timer_result)
		{
			s.cancel();
		}
	}
	
	if (err_result)
	{
		ec = *err_result;
	}
	
	return bytesTransferred;
}

template <typename SyncReadStream, typename MutableBufferSequence>
std::size_t writeWithTimeout(SyncReadStream& s, const MutableBufferSequence& buffers,
	const boost::asio::deadline_timer::duration_type& expiry_time, boost::system::error_code& ec)
{
	std::size_t bytesTransferred = 0;
	boost::optional<boost::system::error_code> timer_result, err_result = boost::none;
	boost::asio::deadline_timer timer(s.get_io_service());
	timer.expires_from_now(expiry_time);
	
	timer.async_wait([&timer_result] (const boost::system::error_code& error)
	{
		timer_result.reset(error);
	});
	
	boost::asio::async_write(s, buffers, [&err_result, &bytesTransferred] (const auto& error, auto bytes_transferred)
	{
		bytesTransferred = bytes_transferred;
		err_result.reset(error);
	});
	
	s.get_io_service().reset();
	while (s.get_io_service().run_one())
	{
		if (err_result)
		{
			timer.cancel();
		}
		else if (timer_result)
		{
			s.cancel();
		}
	}
	
	if (err_result)
	{
		ec = *err_result;
	}
	
	return bytesTransferred;
}

BaseSocket::BaseSocket()
	: innerSocket_(io_service_)
	, address_("")
	, port_("")
{
}

BaseSocket::~BaseSocket()
{
	close();
}

bool BaseSocket::checkError(const boost::system::error_code& ec)
{
	const auto shouldCloseTheSocket =
		ec == boost::asio::error::connection_aborted
			|| 	ec == boost::asio::error::connection_reset
			|| 	ec == boost::asio::error::connection_refused
			|| 	ec == boost::asio::error::operation_aborted
            || 	ec == boost::asio::error::operation_not_supported
			|| 	ec == boost::asio::error::timed_out
			|| 	ec == boost::asio::error::eof
			||	ec == boost::asio::error::broken_pipe
	;
	
	if (ec == boost::asio::error::connection_reset
			|| 	ec == boost::asio::error::operation_aborted
			|| 	ec == boost::asio::error::timed_out
			|| 	ec == boost::asio::error::eof
			||	ec == boost::asio::error::broken_pipe)
	{
		multicastDelegate_([](auto d) { d->reconnectionRequired(); });
	}
	
	if (shouldCloseTheSocket)
	{
		LOG_FATAL(str(boost::format("Fatal error: %1%") % ec.message()).c_str());
		close();
	}

	return shouldCloseTheSocket;
}

bool BaseSocket::isOpen() const
{
	return innerSocket_.is_open();
}

void BaseSocket::close(bool andJoin)
{
	if (isOpen())
	{
		LOG_DEBUG("Closing the socket...");
		multicastDelegate_([](auto d) { d->socketWillClose(); });
		
		innerSocket_.close();
		LOG_INFO("Socket closed");
		
		multicastDelegate_([](auto d) { d->socketDidClose(); });

		if (andJoin)
		{
			if (boost::this_thread::get_id() != thread_.get_id())
			{
				if (thread_.joinable())
				{
					LOG_DEBUG("Socket thread joined successfully");
					thread_.join();
				}
			}
			else
			{
				if (thread_.joinable())
				{
					LOG_DEBUG("Socket thread detached successfully");
					thread_.detach();
				}
			}
		}
		
		npmD_.stop();
		npmU_.stop();

		io_service_.stop();
		io_service_.reset();
	}
}

void BaseSocket::addDelegate(std::shared_ptr<BaseSocketDelegate> delegate)
{
	multicastDelegate_ += delegate;
}

void BaseSocket::removeDelegate(std::shared_ptr<BaseSocketDelegate> delegate)
{
	multicastDelegate_ -= delegate;
}

void BaseSocket::connect(const std::string& address, const std::string& port)
{
	if (address.empty() || port.empty())
	{
		LOG_FATAL(str(boost::format("The socket can't connect to this address: %1%:%2%") % address_ % port_).c_str());
		return;
	}

	// check if the socket is already connected
	close();

	address_ = address;
	port_ = port;

	LOG_DEBUG("Using a multi-threading socket");
	thread_ = boost::thread(boost::bind(&BaseSocket::resolve, this));
}

void BaseSocket::resolve()
{
	using namespace boost::asio::ip;
	tcp::resolver resolver(io_service_);

	LOG_DEBUG(str(boost::format("The socket is attempting to resolve %1%:%2%") % address_ % port_).c_str());
	multicastDelegate_([](auto d) { d->socketWillConnect(); });

	boost::system::error_code ec;
	auto it = resolver.resolve({address_, port_}, ec);
	
	if (!ec)
	{
		LOG_DEBUG(str(boost::format("Address %1%:%2% resolved successfully") % address_ % port_).c_str());
		connectImpl(it);
	}
	else
	{
		auto errMess = str(boost::format("Failed to resolve %1%:%2%, error code: %3%")
			% address_ % port_ % ec.message());
		LOG_DEBUG(errMess.c_str());
		
		multicastDelegate_([errMess, this](const auto d)
		{
			d->socketFailedToConnect(errMess);
			d->reconnectionRequired();
			d->socketDidClose();
		});
	}
}

void BaseSocket::connectImpl(const boost::asio::ip::tcp::resolver::iterator &it)
{
	LOG_DEBUG(str(boost::format("Connecting to %1%:%2%...") % address_ % port_).c_str());
	
	boost::system::error_code ec = boost::asio::error::would_block;
	boost::asio::deadline_timer timer(innerSocket_.get_io_service());
	timer.expires_from_now(socketExpirationTimeout);
	
	using boost::asio::deadline_timer;
	using boost::asio::ip::tcp;
	using boost::lambda::bind;
	using boost::lambda::var;
	using boost::lambda::_1;
	
	boost::asio::async_connect(innerSocket_, it, var(ec) = _1);
	innerSocket_.get_io_service().reset();
	
	do {
		innerSocket_.get_io_service().run_one();
	} while (ec == boost::asio::error::would_block);
	
	if (!ec && innerSocket_.is_open())
	{
		LOG_DEBUG(str(boost::format("Connection on %1%:%2% enstablished successfully") % address_ % port_).c_str());
		socketPostConnection();
		multicastDelegate_([](auto d) { d->socketDidConnect(); });
	}
	else
	{
		auto errMess = boost::str(boost::format("Failed to connect to %1%:%2%, error code: %3%")
			% address_ % port_ % ec.message());
		LOG_DEBUG(errMess.c_str());
		
		multicastDelegate_([errMess](auto d)
		{
			d->socketFailedToConnect(errMess);
			d->reconnectionRequired();
			d->socketDidClose();
		});
	}
}

void BaseSocket::socketPostConnection()
{
}

void BaseSocket::sendData(u8 value,
	const std::function<void(const boost::system::error_code& ec, std::size_t bytes_transferred)> &callback)
{
	auto data = std::make_shared<v8>(1, value);
	sendDataImpl(data, callback);
}

void BaseSocket::sendData(const v8::const_iterator& begin, const v8::const_iterator& end,
	const std::function<void(const boost::system::error_code& ec, std::size_t bytes_transferred)> &callback)
{
	auto data = std::make_shared<v8>(begin, end);
	sendDataImpl(data, callback);
}

void BaseSocket::sendData(const v8& value,
	const std::function<void(const boost::system::error_code& ec, std::size_t bytes_transferred)>& callback)
{
	auto data = std::make_shared<v8>(value);
	sendDataImpl(data, callback);
}

void BaseSocket::sendDataImpl(const std::shared_ptr<v8>& rawData, 
	const std::function<void(const boost::system::error_code& ec, std::size_t bytes_transferred)>& callback)
{
	boost::system::error_code ec;
	
	auto bytes_transferred = writeWithTimeout(innerSocket_, boost::asio::buffer(*rawData), socketExpirationTimeout, ec);
	npmU_.updateData(bytes_transferred);
	
	checkError(ec);
	callback(ec, bytes_transferred);
}

void BaseSocket::receiveData(std::size_t size,
	const std::function<void(const boost::system::error_code& ec,
		std::size_t bytes_transferred, std::shared_ptr<v8> data)>& callback)
{
	auto rawData = std::make_shared<v8>(size);
	auto buffer = boost::asio::buffer(*rawData);
	boost::system::error_code ec;
	
	auto bytesTransferred = readWithTimeout(innerSocket_, buffer, socketExpirationTimeout, size, ec);
	npmD_.updateData(bytesTransferred);
	
	checkError(ec);
    callback(ec, bytesTransferred, rawData);
}
