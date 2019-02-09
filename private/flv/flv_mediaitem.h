//
//  flv_mediaitem.h
//
//  Created by Diego Stamigni on 05/09/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/mediaitem.h"
#include "../../private/multicast_delegate.h"
#include "../../private/flv/flv_mediaheader.h"
#include "../../private/flv/flv_amf_mediatag.h"
#include "../../private/flv/flv_audio_mediatag.h"
#include "../../private/flv/flv_video_mediatag.h"

namespace RtmpKit
{
	struct FLVMediaItemParserDelegate
	{
		virtual ~FLVMediaItemParserDelegate() = default;

		virtual void amfPacketReady(FLVMediaAMFTag tag) = 0;

		virtual void audioPacketReady(FLVMediaAudioTag tag) = 0;

		virtual void videoPacketReady(FLVMediaVideoTag tag) = 0;

		virtual void parsingCompleted() = 0;
	};

	class FLVMediaItem : public MediaItem
	{
	public:
		FLVMediaItem() = default;

		FLVMediaItem(const FLVMediaItem& cp) = default;

		FLVMediaItem(FLVMediaItem&& mv) = default;

		FLVMediaItem& operator=(const FLVMediaItem& cp) = default;

		FLVMediaItem& operator=(FLVMediaItem&& mv) = default;

		virtual void setHeader(const FLVMediaHeader& h)
		{
			this->header_ = h;
		}

		virtual const FLVMediaHeader& header() const
		{
			return this->header_;
		}

		virtual const std::vector<FLVMediaAMFTag>& amfTags() const
		{
			return amfTags_;
		}

		virtual const std::vector<FLVMediaVideoTag>& videoTags() const
		{
			return videoTags_;
		}

		virtual const std::vector<FLVMediaAudioTag>& audioTags() const
		{
			return audioTags_;
		}

		virtual void appendPayload(v8&& p) override;

		virtual void deserialize() override;

		virtual void deserialize(const v8::const_iterator& begin, const v8::const_iterator& end) override;

		void addDelegate(std::shared_ptr<FLVMediaItemParserDelegate> delegate);
		
		void removeDelegate(std::shared_ptr<FLVMediaItemParserDelegate> delegate);

	protected:
		u32 parseHeader();

		void parseTags(const v8& buffer);

		u24 parseNextTags(const v8::const_iterator& begin, const v8::const_iterator& end);

		u32 parseAMFTag(const v8::const_iterator& begin, const v8::const_iterator& end);

		u32 parseVideoTag(const v8::const_iterator& begin, const v8::const_iterator& end);

		u32 parseAudioTag(const v8::const_iterator& begin, const v8::const_iterator& end);

	private:
		FLVMediaHeader header_;

		std::vector<FLVMediaAMFTag> amfTags_;
		std::vector<FLVMediaAudioTag> audioTags_;
		std::vector<FLVMediaVideoTag> videoTags_;

		v8 buffer_;

		// The delegate will notify the client when a packed is parsed
		MulticastDelegate<FLVMediaItemParserDelegate> multicastDelegate_;
	};
} //end RtmpKit
