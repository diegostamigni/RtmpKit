//
//  flv_types.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 7/10/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "../../private/utils.h"

namespace RtmpKit
{
	enum class FLVVideoCodecType : u8
	{
		Sorenses_H263 = 2,
		ScreenVideo = 3,
		On2_VP6 = 4,
		H264 = 7,
	};

	enum class FLVVideoFrameType : u8
	{
		Unknown = 0,
		KeyFrame = 1,
		InterFrame = 2,
		DisposableInterFrame = 3,
	};

	enum class FLVSoundFormatType: u8
	{
		LinearPCM_BE = 0,
		ADPCM = 1,
		MP3 = 2,
		LinearPCM_LE = 3,
		Nellymoser_16kHz = 4,
		Nellymoser_8kHz = 5,
		Nellymoser = 6,
		G711_AlawLogPCM = 7,
		G711_MulawLogPCM = 8,
		Reserved = 9,
		AAC = 10,
		Speex = 11,
		MP3_8kHz = 14,
		DeviceSpecificSound = 15,
	};

	enum class FLVSoundRateType: u8
	{
		_5_5kHz = 0,
		_11kHz = 1,
		_22kHz = 2,
		_44kHz = 3,
	};

	enum class FLVSoundSampleSizeType: u8
	{
		_8bit = 0,
		_16bit = 1,
	};

	enum class FLVSoundType: u8
	{
		Mono = 0,
		Stereo = 1,
	};

	enum class FLVAACPacketType: u8
	{
		Unknown = 99,
		AACSequenceHeader = 0,
		AACRaw = 1,
	};
	
} // end RtmpKit
