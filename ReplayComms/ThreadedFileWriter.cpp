#include "pch.h"
#include "ThreadedFileWriter.hpp"
#include "CircularSpan.h"
#include <iostream>

ThreadedFileWriter::ThreadedFileWriter(size_t buffSize, std::string fileName)
	: _outFile(fileName, std::ios::binary),
	_buffer(buffSize, [this](CircularSpan<std::byte> span) { writeToFile(span); })
{
	if (!_outFile)
	{
		throw std::ios_base::failure("failed to open file to write audio data");
	}
};

ThreadedFileWriter::~ThreadedFileWriter() {
}

void writeToOstream(std::ostream& os, std::span<const std::byte> span)
{
	os.write(reinterpret_cast<const char*>(span.data()), span.size());
}

void ThreadedFileWriter::writeToFile(CircularSpan<std::byte> span)
{
	writeToOstream(_outFile, span._firstSection);
	writeToOstream(_outFile, span._secondSection);
	if (_outFile.bad())
	{
		// TODO error handling
	}
}
