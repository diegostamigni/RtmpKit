#include "../../private/flv/flv_mediatag.h"
using namespace RtmpKit;

void FLVMediaTag::deserialize()
{
}

void FLVMediaTag::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	auto iter = begin;

	setPreviousTagSize(fromBigEndian32({ iter, iter + 4}));
	iter += 4;

	setType(static_cast<FLVTagType>(*iter));
	iter += 1;

	setPayloadSize(fromBigEndian24({iter, iter + 3}));
	iter += 3;

	setTimestamp(fromBigEndian24({iter, iter + 3}));
	iter += 4;

	setStreamID(fromBigEndian24({iter, iter + 3}));
	iter += 3;

	setPayload({iter, iter + payloadSize()});
}
