#include "../private/mediatag.h"
using namespace RtmpKit;

const v8& MediaTag::payload() const
{
	return payload_;
}

void MediaTag::setPayload(const v8& p)
{
	this->payload_ = p;
}

void MediaTag::setPayload(v8&& p)
{
	this->payload_ = std::move(p);
}

const v8& MediaTag::serialize()
{
	return payload_;
}

std::size_t MediaTag::size() const
{
	return payloadSize();
}
