//
//  reader.h
//  Diego Stamigni
//
//  Created by Diego Stamigni on 12/08/2016.
//  Copyright © 2016 Diego Stamigni. All rights reserved.
//

#pragma once

#include "filetype.h"
#include "../private/utils.h"

#include <fstream>
#include <vector>
#include <utility>

namespace RtmpKit
{
	class Reader
	{
	public:
		virtual ~Reader() = default;

		virtual bool eof() const = 0;

		virtual void close() = 0;

		virtual std::streamsize read(char* buffer, std::size_t size) = 0;

		virtual std::pair<std::streamsize, v8> read(std::streamsize numberOfBytes = - 1) = 0;

		virtual std::pair<std::streamsize, v8> readAll() = 0;

		virtual std::streamsize size() = 0;

		virtual void seek(std::streampos off, std::ios_base::seekdir dir) = 0;

		virtual void seek(std::streampos pos) = 0;

		virtual std::streampos currentPosition() = 0;

		virtual FileType type() = 0;
	};

	class BufferReader : public Reader
	{
	public:
		BufferReader() = default;

		BufferReader(BufferReader&& mv) = default;

		BufferReader(const BufferReader& cp) = default;

		BufferReader& operator=(BufferReader&& mv) = default;

		BufferReader& operator=(const BufferReader& cp) = default;
	};

	class FileReader : public Reader
	{
	public:
		FileReader() = default;

		FileReader(FileReader&& mv) = default;

		FileReader(const FileReader& cp) = default;

		FileReader& operator=(FileReader&& mv) = default;

		FileReader& operator=(const FileReader& cp) = default;

		explicit FileReader(const std::string& path);

		explicit FileReader(const char* path);

		virtual std::streamsize size() override;

		virtual bool eof() const override;

		virtual void close() override;

		virtual std::streamsize read(char* buffer, std::size_t size) override;

		virtual std::pair<std::streamsize, v8> read(std::streamsize numberOfBytes = - 1) override;

		virtual std::pair<std::streamsize, v8> readAll() override;

		virtual void seek(std::streampos off, std::ios_base::seekdir dir) override;

		virtual void seek(std::streampos pos) override;

		virtual std::streampos currentPosition() override;

		virtual FileType type() override;

		bool exist() const;

		bool isOpen() const;

	private:
		std::ifstream stream;
	};
} // end RtmpKit