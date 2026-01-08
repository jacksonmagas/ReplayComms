#include "pch.h"
#include "TwoThreadRingBuffer.h"
#include "CircularSpan.h"
#include <iostream>

TwoThreadRingBuffer::TwoThreadRingBuffer(size_t buffSize, std::string fileName)
	: _outFile(fileName, std::ios::binary),
	_buffer(buffSize + 1),
	_pendingDataBegin({ _buffer.size(), 0}),
	_pendingDataEnd({ _buffer.size(), 0}),
	_writePending(false),
	_writeThread("Write worker thread", &TwoThreadRingBuffer::fileWriteWorker, this)
{
	if (!_outFile)
	{
		throw std::ios_base::failure("failed to open file to write audio data");
	}
	InvariantGuard g(this);
};

TwoThreadRingBuffer::~TwoThreadRingBuffer() {
	{
		InvariantGuard g(this);
	}
	_writeThread.markForShutDown();
	{
		std::lock_guard lock(_writePendingMutex);
		_writePending = true;
	}
	_newData.notify_one();
	_writeThread.join();

	const ModularIndex beginIndex = _pendingDataBegin.load();
	const ModularIndex endIndex   = _pendingDataEnd.load();
	bool bufferEmpty = beginIndex == endIndex;
	if (!bufferEmpty)
	{
		writeToFile(beginIndex, endIndex);
	}
}

// reads both indexes, modifies only end.
std::span<const std::byte> TwoThreadRingBuffer::addData(std::span<const std::byte> data)
{
	InvariantGuard g(this);
	ModularIndex endIndex = _pendingDataEnd.load();
	const ModularIndex beginIndex = _pendingDataBegin.load();

	//_outFile << "Adding data to writable section from " << endIndex << " to " << beginIndex << std::endl;
	CircularSpan<std::byte> writableSection(_buffer, endIndex, beginIndex - std::ptrdiff_t{ 1 });
	const size_t amountCopiedFirstSection = std::min(data.size(), writableSection._firstSection.size());
	const size_t amountCopiedSecondSection = std::min(data.size() - amountCopiedFirstSection, writableSection._secondSection.size());
	auto dataStart = data.begin();
	auto splitPoint = dataStart + amountCopiedFirstSection;
	auto secondSectionEnd = splitPoint + amountCopiedSecondSection;
	std::copy(dataStart, splitPoint, writableSection._firstSection.begin());
	std::copy(splitPoint, secondSectionEnd, writableSection._secondSection.begin());

	endIndex += amountCopiedFirstSection + amountCopiedSecondSection;
	//_outFile << "Added " << amountCopiedFirstSection + amountCopiedSecondSection << " bytes with new end index at " << endIndex << std::endl;
	_pendingDataEnd.store(endIndex);
	{
		std::lock_guard lock(_writePendingMutex);
		_writePending = true;
	}
	_newData.notify_one();
	return data.subspan(amountCopiedFirstSection + amountCopiedSecondSection);
}

void writeToOstream(std::ostream& os, std::span<const std::byte> span)
{
	os.write(reinterpret_cast<const char*>(span.data()), span.size());
}

ModularIndex<> TwoThreadRingBuffer::writeToFile(ModularIndex<> beginIndex, ModularIndex<> previousEndIndex) const
{
	size_t readSize = previousEndIndex - beginIndex;
	if (beginIndex != previousEndIndex)
	{
		auto readableSection = CircularSpan<std::byte>::fromSizeConst(_buffer, beginIndex, readSize);
		writeToOstream(_outFile, readableSection._firstSection);
		writeToOstream(_outFile, readableSection._secondSection);
		if (_outFile.bad())
		{
			// TODO error handling
		}
		return previousEndIndex;
	}
	return beginIndex;
}

// reads both indexes, modifies only start.
void TwoThreadRingBuffer::fileWriteWorker()
{
	InvariantGuard g(this);
	{
		std::unique_lock lock(_writePendingMutex);
		using namespace std::chrono_literals;
		_newData.wait(lock, [this]() { return _writePending;  });
		_writePending = false;
	}

	const ModularIndex previousEndIndex = _pendingDataEnd.load();
	const ModularIndex beginIndex = _pendingDataBegin.load();
	const ModularIndex newBeginIndex = writeToFile(beginIndex, previousEndIndex);
	_pendingDataBegin = newBeginIndex;
}
