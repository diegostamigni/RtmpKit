//
//  mediaheader.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "parsable.h"

namespace RtmpKit
{
	enum class MediaItemFlags : u8
	{
		Video = 0x01,
		Audio = 0x04,
		AudioVideo = 0x05,
		Unknown = 0x00,
	};

	class MediaHeader: public Parsable
	{
	public:
		MediaHeader() = default;

		MediaHeader(const MediaHeader& cp) = default;

		MediaHeader(MediaHeader&& mv) = default;

		MediaHeader& operator=(const MediaHeader& cp) = default;

		MediaHeader& operator=(MediaHeader&& mv) = default;

		virtual const std::string &signature() const = 0;

		virtual void setSignature(const std::string& h) = 0;

		virtual u32 size() const = 0;

		virtual void setSize(u32 h) = 0;

		virtual int version() const = 0;

		virtual void setVersion(int v) = 0;

		virtual MediaItemFlags flags() const = 0;

		virtual void setFlags(MediaItemFlags t) = 0;
	};
} // end RtmpKit