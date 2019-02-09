#include "../../private/flv/flv_mediaitem.h"
using namespace RtmpKit;

void FLVMediaItem::appendPayload(v8&& data)
{
    if (data.empty()) return;
    std::move(data.begin(), data.end(), std::back_inserter(buffer_));
}

void FLVMediaItem::deserialize()
{
	auto headerSize = parseHeader();
	parseTags({buffer_.cbegin() + headerSize, buffer_.cend()});
	buffer_.clear();
}

void FLVMediaItem::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	buffer_ = {begin, end};
	deserialize();
}

u32 FLVMediaItem::parseHeader()
{
	header_.deserialize(buffer_.cbegin(), buffer_.cend());
	return header_.size();
}

void FLVMediaItem::parseTags(const v8& buffer)
{
	auto nextIter = buffer.cbegin();
	auto endIter = buffer.cend();
	auto validCursor = std::distance(nextIter, endIter) > 0;

	while (validCursor || nextIter < buffer.end())
	{
		auto nextInsertPos = parseNextTags(nextIter, endIter);

		if (nextIter == buffer.cend() 
			|| std::distance(nextIter, buffer.cend()) < static_cast<v8::const_iterator::difference_type>(nextInsertPos))
			break;

		nextIter += nextInsertPos;
		validCursor = std::distance(nextIter, endIter) > 0;
	}

	multicastDelegate_([this](const auto &d)
	{
		d->parsingCompleted();
	});
}

u24 FLVMediaItem::parseNextTags(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	u32 size = 1;
	const auto distance = std::distance(begin, end);

	if (distance <= 4) return size;
	auto tagType = FLVMediaTag::tagType(*(begin + 4));

	switch (tagType)
	{
	case FLVTagType::AMFMetadata:
	{
		size = parseAMFTag(begin, end);
		break;
	}

	case FLVTagType::AudioPayload:
	{
		size = parseAudioTag(begin, end);
		break;
	}

	case FLVTagType::VideoPayload:
	{
		size = parseVideoTag(begin, end);
		break;
	}

	default:
		break;
	}

	return size;
}

u24 FLVMediaItem::parseAMFTag(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	auto tag = FLVMediaAMFTag{};
	tag.deserialize(begin, end);

	auto size = tag.payloadSizeAndHeader();
	amfTags_.push_back(std::move(tag));

	multicastDelegate_([this](const auto &d)
	{
		d->amfPacketReady(amfTags_.back());
	});

	return size;
}

u24 FLVMediaItem::parseAudioTag(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	auto tag = FLVMediaAudioTag{};
	tag.deserialize(begin, end);

	auto size = tag.payloadSizeAndHeader();
	audioTags_.push_back(std::move(tag));

	multicastDelegate_([this](const auto &d)
	{
		d->audioPacketReady(audioTags_.back());
	});

	return size;
}

u24 FLVMediaItem::parseVideoTag(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	auto tag = FLVMediaVideoTag{};
	tag.deserialize(begin, end);

	auto size = tag.payloadSizeAndHeader();
	videoTags_.push_back(std::move(tag));

	multicastDelegate_([this](const auto &d)
	{
		d->videoPacketReady(videoTags_.back());
	});

	return size;
}

void FLVMediaItem::addDelegate(std::shared_ptr<FLVMediaItemParserDelegate> delegate)
{
	multicastDelegate_ += delegate;
}

void FLVMediaItem::removeDelegate(std::shared_ptr<FLVMediaItemParserDelegate> delegate)
{
	multicastDelegate_ -= delegate;
}
