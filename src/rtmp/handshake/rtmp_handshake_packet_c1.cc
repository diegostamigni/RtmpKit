#include "../../../private/rtmp/handshake/rtmp_handshake_packet_c1.h"
#include "../../../private/rtmp/rtmp_utils.h"

#include <boost/random.hpp>

using namespace RtmpKit;
static constexpr const std::size_t C1_S1_PacketSize = 1536;

RtmpHandshakePacketC1::RtmpHandshakePacketC1()
	: RtmpHandshakePacket()
	, data_(C1_S1_PacketSize)
	, epoc_(0ms)
	, time_(RtmpKit::toBigEndian32(static_cast<u32>(epoc_.count())))
	, zeros_(RtmpKit::v8(4))
	, randomBytes_(RtmpKit::v8(RtmpHandshakePacketC1::defaultSize() - time_.size() - zeros_.size()))
{
}

const v8& RtmpHandshakePacketC1::serialize()
{
	data_.clear();

	time_ = RtmpKit::toBigEndian32(static_cast<u32>(epoc_.count()));
	zeros_ = RtmpKit::v8(4);
	randomBytes_ = RtmpKit::v8(size() - time_.size() - zeros_.size());

	// generates random data in randomBytes
	boost::random::mt19937 generator{static_cast<RtmpKit::u32>(::time(nullptr))};
	generator.generate(randomBytes_.begin(), randomBytes_.end());

	std::move(time_.begin(), time_.end(), back_inserter(data_));
	std::move(zeros_.begin(), zeros_.end(), back_inserter(data_));
	std::move(randomBytes_.begin(), randomBytes_.end(), back_inserter(data_));

	return data_;
}

void RtmpHandshakePacketC1::deserialize()
{
}

void RtmpHandshakePacketC1::deserialize(const v8::const_iterator& begin, const v8::const_iterator& end)
{
	if (begin == end)
	{
		return;
	}

	auto it = begin;

	auto size = time_.size();
	time_ = v8{it, it + size};
	it += size;

	size = zeros_.size();
	zeros_ = v8{it, it + size};
	it += size;

	randomBytes_ = v8{it, end};
}

std::size_t RtmpHandshakePacketC1::size() const
{
	return RtmpHandshakePacketC1::defaultSize();
}

std::size_t RtmpHandshakePacketC1::defaultSize()
{
	return C1_S1_PacketSize;
}
