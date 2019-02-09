#include "../../../private/rtmp/handshake/rtmp_handshake_packet_c2.h"

#include <boost/random.hpp>
using namespace RtmpKit;

RtmpHandshakePacketC2::RtmpHandshakePacketC2(const RtmpHandshakePacketS1& cp)
	: RtmpHandshakePacketS1(cp)
{
	epoc_ = RtmpKit::currentTimeMillis();
}

const v8& RtmpHandshakePacketC2::serialize()
{
	data_.clear();

	zeros_ = RtmpKit::toBigEndian32(static_cast<u32>(epoc_.count()));
	randomBytes_ = RtmpKit::v8(size() - time_.size() - zeros_.size());

	// generates random data in randomBytes
	boost::random::mt19937 generator{static_cast<RtmpKit::u32>(::time(nullptr))};
	generator.generate(randomBytes_.begin(), randomBytes_.end());

	std::move(time_.begin(), time_.end(), back_inserter(data_));
	std::move(zeros_.begin(), zeros_.end(), back_inserter(data_));
	std::move(randomBytes_.begin(), randomBytes_.end(), back_inserter(data_));

	return data_;
}
