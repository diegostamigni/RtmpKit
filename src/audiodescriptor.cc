#include "../private/audiodescriptor.h"
using namespace RtmpKit;

int AudioDescriptor::channelCount() const
{
	return this->channelCount_;
}

void AudioDescriptor::setChannelCount(int value)
{
	this->channelCount_ = value;
}

int AudioDescriptor::frameLength() const
{
	return this->frameLength_;
}

void AudioDescriptor::setFrameLength(int value)
{
	this->frameLength_ = value;
}

int AudioDescriptor::sampleRate() const
{
	return this->sampleRate_;
}

void AudioDescriptor::setSampleRate(int value)
{
	this->sampleRate_ = value;
}

bool AudioDescriptor::isValid() const
{
	return channelCount_ > 0 && frameLength_ > 0;
}
