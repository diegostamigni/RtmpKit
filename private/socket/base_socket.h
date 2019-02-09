//
//  basic_socket.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 21/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/network_performance_metrics.h"
#include "../../private/multicast_delegate.h"
#include "../../private/utils.h"

#include <functional>
#include <boost/asio.hpp>
#include <memory>
#include <boost/thread.hpp>

namespace RtmpKit
{
	class BaseSocketDelegate
	{
	public:
		/*!
		 * @brief Fired when the connection is going to be opened.
		 */
		virtual void socketWillConnect() = 0;

		/*!
		 * @brief Fired when the connection has been created and the socket is ready.
		 */
		virtual void socketDidConnect() = 0;

		/*!
		 * @brief Fired when the connection is going to be closed.
		 */
		virtual void socketWillClose() = 0;

		/*!
		 * @brief Fired when the connection has been closed.
		 */
		virtual void socketDidClose() = 0;

		/*!
		 * @brief Fired when the connection failed to start.
		 * @param errMessage is the error message, useful for debug information
		 */
		virtual void socketFailedToConnect(const std::string& errMessage) = 0;
		
		/*!
		 * @brief Fired when a reconnection is required
		 */
		virtual void reconnectionRequired() = 0;

		virtual ~BaseSocketDelegate() = default;
	};

	class BaseSocket
	{
	public:
		BaseSocket();
		virtual ~BaseSocket();

		BaseSocket(const BaseSocket &) = delete;
		BaseSocket& operator=(const BaseSocket &) = delete;

		/*!
		 * @return True if the socket is open
		 */
		bool isOpen() const;

		/*! Async close
		 *
		 * @brief This method will asynchronously close the socket.
		 */
		void close(bool andJoin = true);

		/*! Async connect
		 *
		 * @brief This method will asynchronously connect the socket to a specified address and port.
		 * @param address The address of the host
		 * @param port The where to connect to
		 */
		virtual void connect(const std::string& address, const std::string& port);

		/*! Add a delegate to the socket in order to handle its state.
		 * @param delegate the delegate that will listen to
		 */
		virtual void addDelegate(std::shared_ptr<BaseSocketDelegate> delegate);
		
		/*! Remove a delegate from the socket
		 * @param delegate the delegate that must be removed
		 */
		virtual void removeDelegate(std::shared_ptr<BaseSocketDelegate> delegate);

		/*!
		 * @return the current address used by the socket
		 
		 */
		const std::string& address() const { return address_; }

		/*!
		 * @return the current port used by the socket
		 */
		const std::string& port() const { return port_; }

		/*!
		 * Send data throught the socket.
		 * @param data an octect in bit to send throught the socket
		 * @param callback the callback to call to when the async send has done
		 */
		void sendData(u8 data,
			const std::function<void(const boost::system::error_code& ec, std::size_t bytes_transferred)>& callback);

		/*!
		 * Send data throught the socket.
		 * @param begin the begin const iterator of a byte array
		 * @param end the begin const iterator of a byte array
		 * @param callback the callback to call to when the async send has done
		 */
		void sendData(const v8::const_iterator &begin, const v8::const_iterator &end,
			const std::function<void(const boost::system::error_code& ec, std::size_t bytes_transferred)> &callback);

		/*!
		 * Send data throught the socket.
		 * @param data is a byte array
		 * @param callback the callback to call to when the async send has done
		 */
		void sendData(const v8 &data,
			const std::function<void(const boost::system::error_code& ec, std::size_t bytes_transferred)> &callback);

		/*!
		 * Receive data from the socket.
		 * @param size How many data you want to read from the socket
		 * @param callback the handler
		 */
		void receiveData(std::size_t size,
			const std::function<void(const boost::system::error_code& ec,
				std::size_t bytes_transferred, std::shared_ptr<v8> data)>& callback);
		
		/*!
		 * @return Returns the current download NetworkPerformanceMetrics object
		 */
		NetworkPerformanceMetrics& downloadingNetworkPerformanceMetrics()
		{
			return npmD_;
		}
		
		/*!
		 * @return Returns the current upload NetworkPerformanceMetrics object
		 */
		NetworkPerformanceMetrics& uploadingNetworkPerformanceMetrics()
		{
			return npmU_;
		}

	protected:
		virtual bool checkError(const boost::system::error_code& ec);
		virtual void resolve();
		virtual void connectImpl(const boost::asio::ip::tcp::resolver::iterator &it);
		virtual void socketPostConnection();
		void sendDataImpl(const std::shared_ptr<v8> &rawData,
			const std::function<void(const boost::system::error_code& ec, std::size_t bytes_transferred)> &callback);
		
		boost::asio::io_service io_service_;
		boost::asio::ip::tcp::socket innerSocket_;
		boost::thread thread_;

	private:
		std::string address_;
		std::string port_;

	private:
		NetworkPerformanceMetrics npmD_;
		NetworkPerformanceMetrics npmU_;
		MulticastDelegate<BaseSocketDelegate> multicastDelegate_;
	};
} // end RtmpKit
