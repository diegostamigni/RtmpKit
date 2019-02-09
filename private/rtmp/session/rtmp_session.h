//
//  rtmp_session.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 27/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../private/rtmp/rtmp_socket.h"
#include "../../../private/rtmp/rtmp_utils.h"

#include <memory>

namespace RtmpKit
{
	struct RtmpSessionDelegate : public BaseSocketDelegate
	{
		virtual void handshakeDone() = 0;

		virtual void sessionStarted() = 0;
	};

	class RtmpSession : public BaseSocketDelegate,
		public NetworkPerformanceMetricsDelegate,
		public std::enable_shared_from_this<RtmpSession>
	{
	public:
		RtmpSession();
		RtmpSession(RtmpSession&&) = default;
		RtmpSession& operator=(RtmpSession&&) = default;

		RtmpSession(const RtmpSession&) = delete;
		RtmpSession& operator=(const RtmpSession&) = delete;

		/*!
		 * Register the delegate in order to receive notifications about the status of the session
		 * @param delegate
		 */
		virtual void addDelegate(std::shared_ptr<RtmpSessionDelegate> delegate);
		
		/*!
		 * Remove the delegate from the current session
		 * @param delegate
		 */
		virtual void removeDelegate(std::shared_ptr<RtmpSessionDelegate> delegate);
		
		/*!
		 * Starts the RtmpSession, if it hasn't already
		 * @param address
		 * @param port
		 * @param streamId
		 */
		virtual void start(const std::string& address, const std::string& port, const std::string& streamId);
		
		/*!
		 * Starts the RtmpSession, if it hasn't already
		 * @param address
		 * @param port
		 * @param appName
		 * @param stream
		 */
		virtual void start(const std::string& address, const std::string& port,
			const std::string& appName, const std::string& stream);
		
		/*!
		 * Ends the current session if running
		 */
		virtual void end();

		/*!
		 * @return the current streaming appName
		 */
		const std::string& appName() const { return appName_; }

		/*!
		 * @return the current streaming ID
		 */
		const std::string& streamID() const { return streamName_; }

		/*!
		 * @return the host currently used by the socket
		 */
		const std::string& address() const;

		/*!
		 * @return the port currently used by the socket
		 */
		const std::string& port() const;

		/*!
		 * RTMP Handshake
		 * @brief Send the handshake via RTMP.
		 * @details [5.2.1.]  Handshake Sequence
		 * @details Adobe RTMP ([rtmp_specification_1.0.pdf] December 2012)
		 */
		virtual void handshake();

		/*!
		 * RTMP Version
		 *
		 * The version defined by this specification is 3.  Values 0-2 are deprecated values used by earlier
		 * proprietary products; 4-31 are reserved for future implementations; and 32-255 are not allowed
		 * (to allow distinguishing RTMP from text-based protocols, which always start with a printable character).
		 * A server that does not recognize the client’s requested version SHOULD respond with 3.
		 * The client MAY choose to degrade to version 3, or to abandon the handshake.

		 * @return the version of the current RTMP implementation.
		 */
		u8 version();
		
		/*!
		 * Restart the current metrics with a new byte rate bound
		 */
		void updateCurrentMetrics(std::size_t currentByteRate);
		
		/*!
		 * Will clear the data queues (like buffers).
		 */
		virtual void reset();
		
	protected:
		virtual void extractAppNameAndStreamID(const std::string& stream);

		virtual void handshakeCompleted();
		
		virtual void sessionReady();
		
		virtual void sendCloseStream();

		virtual void socketWillConnect() override;

		virtual void socketDidConnect() override;

		virtual void socketWillClose() override;

		virtual void socketDidClose() override;

		virtual void socketFailedToConnect(const std::string& errMessage) override;
		
		virtual void reconnectionRequired() override;
		
	protected:
		virtual void networkMetricsChanged(NetworkPerformanceMetrics& sender,
			NetworkType type, std::size_t preferredByterate) override;
		virtual void networkMetricsUpgraded(NetworkPerformanceMetrics& sender,
			NetworkType type, std::size_t preferredByterate) override;
		virtual void networkMetricsDowngraded(NetworkPerformanceMetrics& sender,
			NetworkType type, std::size_t preferredByterate) override;

	protected:
		std::unique_ptr<RtmpSocket> socket_;
		std::string appName_;
		std::string streamName_;
		MulticastDelegate<RtmpSessionDelegate> multicastDelegate_;
	};
} // end RtmpKit
