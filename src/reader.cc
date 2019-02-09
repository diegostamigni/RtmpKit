#include "../private/reader.h"
#include "../private/constants.h"

#include <boost/algorithm/string.hpp>

using namespace RtmpKit;

FileReader::FileReader(const std::string& path)
	: Reader(), stream{path, std::ios::binary | std::ios::in}
{
}

FileReader::FileReader(const char* path)
	: FileReader{std::string{path}}
{
}

bool FileReader::eof() const
{
	return isOpen() ? stream.eof() : true;
}

void FileReader::close()
{
	if (isOpen())
	{
		stream.close();
	}
}

std::streamsize FileReader::read(char* buffer, std::size_t size)
{
	if (! eof())
	{
		stream.read(buffer, size);
	}

	return stream.gcount();
}

std::pair<std::streamsize, v8> FileReader::read(std::streamsize numberOfBytes)
{
	auto buffer = v8(
		(unsigned long) (numberOfBytes >= 0 ? numberOfBytes : RtmpKit::videoReadBufferSize), 0);
	std::streamsize bytesRead = 0;

	while (bytesRead != numberOfBytes && ! eof())
	{
		stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
		bytesRead += stream.gcount();
	}

	return std::make_pair(bytesRead, buffer);
}

std::pair<std::streamsize, v8> FileReader::readAll()
{
	return read(size());
}

std::streamsize FileReader::size()
{
	if (! isOpen())
	{
		return 0;
	}

	auto fpos = currentPosition();
	seek(0, stream.end);

	auto length = currentPosition();
	seek(fpos);

	return length;
}

void FileReader::seek(std::streampos off, std::ios_base::seekdir dir)
{
	stream.seekg(off, dir);
}

void FileReader::seek(std::streampos pos)
{
	stream.seekg(pos);
}

std::streampos FileReader::currentPosition()
{
	return stream.tellg();
}

bool FileReader::exist() const
{
	return isOpen();
}

bool FileReader::isOpen() const
{
	return stream.is_open();
}

FileType FileReader::type()
{
	if (!isOpen())
	{
		return FileType::Unknown;
	}

	auto currentPos = currentPosition();
	seek(0, std::ios::beg);

	// TODO(diegostamigni): Improve
	auto result = read(9); // header size is always 9 for every file flags ?
	seek(currentPos, std::ios::beg);

	if (result.second.size() == 0)
	{
		return FileType::Unknown;
	}

	auto strRepr = std::string(result.second.cbegin(), result.second.cend());
	boost::to_upper(strRepr);

	for (const auto &supportedFileType : RtmpKit::supportedFileTypes)
	{
		if (boost::contains(strRepr, supportedFileType.second))
		{
			return supportedFileType.first;
		}
	}

	return FileType::Unknown;
}