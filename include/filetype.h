//
//  filetype.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 15/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>
#include <map>

namespace RtmpKit
{
	enum class FileType : uint8_t
	{
		FLV = 0x01,
		Unknown = 0x00,
	};

	static const std::map<FileType, std::string> supportedFileTypes =
	{
		{ FileType::Unknown, "Unknown" },
		{ FileType::FLV,     "FLV" },
	};

	static const std::map<std::string, FileType> supportedFileTypesByString
	{
		{ "Unknown",	FileType::Unknown },
		{ "FLV",		FileType::FLV },
	};

	static bool isFileExtensionSupported(const std::string &ext)
	{
		auto &sft = supportedFileTypesByString;
		return sft.find(ext) != sft.end();
	}
} // end RtmpKit
