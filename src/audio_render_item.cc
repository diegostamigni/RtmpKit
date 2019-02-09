#include "../private/audio_render_item.h"
using namespace RtmpKit;

AudioRenderItem::AudioRenderItem(RtmpKit::u24 timestamp, RtmpKit::u24 duration, 
    RtmpKit::u32 bufferLenght, std::shared_ptr<RtmpKit::u8> payload)
    : timestamp_(timestamp)
    , duration_(duration)
    , bufferLenght_(bufferLenght)
    , payload_(payload)
{
}

void AudioRenderItem::setTimestamp(RtmpKit::u24 value)
{
    timestamp_ = value;
}

void AudioRenderItem::setDuration(RtmpKit::u24 value)
{
    duration_ = value;
}

void AudioRenderItem::setBufferLenght(RtmpKit::u32 value)
{
    bufferLenght_ = value;
}

void AudioRenderItem::setPayload(std::shared_ptr<RtmpKit::u8> value)
{
    payload_ = value;
}
