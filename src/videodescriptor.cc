#include "../private/videodescriptor.h"
using namespace RtmpKit;

const RtmpKit::v8& VideoDescriptor::sps() const
{
	return this->sps_;
}

void VideoDescriptor::setSPS(const RtmpKit::v8& value)
{
	this->sps_ = value;
}

void VideoDescriptor::setSPS(RtmpKit::v8&& value)
{
	this->sps_ = value;
}

const RtmpKit::v8& VideoDescriptor::pps() const
{
	return this->pps_;
}

void VideoDescriptor::setPPS(const RtmpKit::v8& value)
{
	this->pps_ = value;
}

void VideoDescriptor::setPPS(RtmpKit::v8&& value)
{
	this->pps_ = value;
}

int32_t VideoDescriptor::nalUnitSize() const
{
	return this->nalUnitSize_;
}

void VideoDescriptor::setNalUnitSize(int32_t value)
{
	this->nalUnitSize_ = value;
}

bool VideoDescriptor::isValid() const
{
	return !this->pps_.empty() && !this->sps_.empty();
}
