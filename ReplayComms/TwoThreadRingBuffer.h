#pragma once

#include <vector>
#include <array>
#include <span>
#include <fstream>
#include <mutex>
#include <atomic>
#include <filesystem>
#include "ThreadWrapper.h"
#include "ModularIndex.h"
#include "CircularSpan.h"
#include "InvariantGuard.h"

static_assert(AtomicModularIndex<>::is_always_lock_free);

// This class provides an interface for writing data to a file in another thread
// Data is provided to a ring buffer and then written to another thread
template<typename T>
class TwoThreadRingBuffer {
public:
	static auto invariants() {
		using Inv = Invariant<TwoThreadRingBuffer>;
		return std::to_array({
			Inv{"Data start in bounds", [](const TwoThreadRingBuffer& t) { return t._pendingDataBegin.load() < t._buffer.size();}},
			Inv{"Data end in bounds", [](const TwoThreadRingBuffer& t) { return t._pendingDataEnd.load() < t._buffer.size();}},
			Inv{"Buffer size positive", [](const TwoThreadRingBuffer& t){ return t._buffer.size() > 0; }},
		});
	}

	// takes the size of the buffer and a function to consume data from the buffer when it is ready
	TwoThreadRingBuffer(size_t buffSize, std::function<void(CircularSpan<T>)> consumerFunc)
	  : _buffer(buffSize + 1),
		_generationCount(),
		_pendingDataBegin({ _buffer.size(), 0}),
		_pendingDataEnd({ _buffer.size(), 0}),
		_stopSource(),
		_consumerThread([this, consumerFunc](std::stop_token stok) {
			std::stop_callback on_stop(stok, [this] {
				++_generationCount;
				_generationCount.notify_all();
			});
			while (true) {
				InvariantGuard g(this);
				if (stok.stop_requested())
				{
					break;
				};
				_generationCount.wait(_previousGenerationCount);
				_generationCount.store(_generationCount.load());
				auto consumedDataEnd = _pendingDataEnd.load();
				consumerFunc({_buffer, _pendingDataBegin.load(), consumedDataEnd});
				_pendingDataBegin.store(consumedDataEnd);
			}
		}, _stopSource.get_token())
	{
		InvariantGuard g(this);
	};

	/**
	* Add the span to the file writer's buffer.
	* If the span is too large adds as much as possible.
	* @return a span containing the remaining data which was not added
	*/
	[[nodiscard]] std::span<const std::byte> addData(std::span<const std::byte> data)
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
		_generationCount++;
		_generationCount.notify_all();
		return data.subspan(amountCopiedFirstSection + amountCopiedSecondSection);
	}

	static constexpr bool is_always_lock_free = AtomicModularIndex<>::is_always_lock_free;

private:
	std::vector<std::byte> _buffer;
	std::atomic<size_t> _generationCount;
	size_t _previousGenerationCount;
	AtomicModularIndex<> _pendingDataBegin; // the range of data that can be consumed is from _pendingDataBegin to _pendingDataEnd
	AtomicModularIndex<> _pendingDataEnd; 
	std::stop_source _stopSource;

	std::jthread _consumerThread;
};
