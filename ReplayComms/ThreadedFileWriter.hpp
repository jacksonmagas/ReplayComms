#include <vector>
#include <array>
#include <span>
#include <fstream>
#include <mutex>
#include <atomic>
#include <filesystem>
#include "ThreadWrapper.h"
#include "ModularIndex.h"
#include "InvariantGuard.h"
#include "TwoThreadRingBuffer.h"

// This class provides an interface for writing data to a file in another thread
// Data is provided to a ring buffer and then written to another thread
struct ThreadedFileWriter {
	ThreadedFileWriter(size_t buffSize, std::string fileName);
	ThreadedFileWriter(size_t buffSize, std::filesystem::path fileName) : ThreadedFileWriter(buffSize, fileName.string()) {};
	// blocks for up to the wait period for write thread to shutdown
	~ThreadedFileWriter();

	void writeToFile(CircularSpan<std::byte> data);

	std::ofstream _outFile;
	TwoThreadRingBuffer<std::byte> _buffer;
};