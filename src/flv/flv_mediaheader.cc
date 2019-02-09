#include <vector>

#include "../../private/flv/flv_mediaheader.h"

using namespace RtmpKit;

FLVMediaHeader::FLVMediaHeader()
	: MediaHeader(), signature_{""}, hSize_{0}, version_{0}, flags_{MediaItemFlags::Unknown}
{
}

void FLVMediaHeader::deserialize()
{
}

void FLVMediaHeader::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	auto iter = begin;

	this->setSignature(std::string(iter, iter + 3));
	iter += 3;
	
	this->setVersion(static_cast<int>(*iter));
	iter += 1;

	this->setFlags(static_cast<MediaItemFlags>(*iter));
	iter += 1;

	this->setSize(fromBigEndian32({ iter, iter + 4 }));
}