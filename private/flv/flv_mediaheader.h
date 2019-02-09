//
//  flv_mediaheader.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/mediaheader.h"

namespace RtmpKit
{
	class FLVMediaHeader : public MediaHeader
	{
	public:
		FLVMediaHeader();

		FLVMediaHeader(const FLVMediaHeader& cp) = default;

		FLVMediaHeader(FLVMediaHeader&& mv) = default;

		FLVMediaHeader& operator=(const FLVMediaHeader& cp) = default;

		FLVMediaHeader& operator=(FLVMediaHeader&& mv) = default;

		virtual const std::string &signature() const override
		{
			return signature_;
		}

		virtual void setSignature(const std::string& h) override
		{
			this->signature_ = h;
		}

		virtual int version() const override
		{
			return version_;
		}

		virtual void setVersion(int v) override
		{
			this->version_ = v;
		}

		virtual MediaItemFlags flags() const override
		{
			return flags_;
		}

		virtual void setFlags(MediaItemFlags t) override
		{
			this->flags_ = t;
		}

		virtual u32 size() const override
		{
			return hSize_;
		}

		virtual void setSize(u32 h) override
		{
			this->hSize_ = h;
		}

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

	protected:
		virtual void deserialize() override;

	private:
		std::string signature_ = "";
		uint32_t hSize_ = 0;
		int version_ = 0;
		MediaItemFlags flags_ = MediaItemFlags::Unknown;
	};
} // end RtmpKit