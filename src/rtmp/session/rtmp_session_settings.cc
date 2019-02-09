#include "../../../private/rtmp/session/rtmp_session_settings.h"
using namespace RtmpKit;

RtmpSessionSettings::RtmpSessionSettings()
	: chunkSize_ { 16384 }
	, videoCodecID_ { FLVVideoCodecType::H264 }
	, soundFormat_ { FLVSoundFormatType::AAC }
	, soundRate_ { FLVSoundRateType::_44kHz }
	, soundSampleSize_ { FLVSoundSampleSizeType::_16bit }
	, soundType_ { FLVSoundType::Stereo }
	, frameRate_ { 30. }
	, audioChannels_ { 1. }
	, videoDataRate_ { 350. * 1024. }
	, videoFrameRate_ { 0. }
	, width_ { 1280. }
	, height_{ 720. }
{
}
