//
//  rtmp_connect_command_message.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 29/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../../../private/rtmp/message/command/rtmp_command_message.h"

namespace RtmpKit
{
	/*!
	 * Represent the RTMP NetConnection command message
	 * @details 7.2.1.1. Adobe RTMP specification doc
	 */
	class RtmpConnectCommandMessage : public RtmpCommandMessage
	{
	public:
		RtmpConnectCommandMessage() = default;

		RtmpConnectCommandMessage(const std::string& address, const std::string& port, const std::string& streamId);

		virtual const v8& serialize() override;

		const AMF::TDictSubset& optionalUserArgumentsProperties() const
		{
			return optionalUserArguments_;
		}

		template <typename K, typename V,
			typename = std::enable_if<std::is_convertible<K, std::string>::value
				&& std::is_convertible<V, AMF::TVariantSubset>::value>>
		void addOptionalUserArgumentsProperty(K&& key, V&& value)
		{
			addProperty(std::forward<K>(key), std::forward<V>(value), optionalUserArguments_);
		}

	protected:
		AMF::TDictSubset optionalUserArguments_;
	};
} // end RtmpKit
