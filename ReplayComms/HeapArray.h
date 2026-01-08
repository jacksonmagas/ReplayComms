#pragma once
#include <memory>
#include <optional>

template<typename ElementType>
class HeapArray
{
public:
	using iterator = ElementType*;
	using const_iterator = const iterator;
	explicit HeapArray(const size_t size) : _size(size), _buffer(std::make_unique<ElementType[]>(size)) {};
	explicit HeapArray(const HeapArray<ElementType>& other) : _size(other._size), _buffer(std::make_unique<ElementType[]>(other._size))
	{
		std::memcpy(_buffer.get(), other._buffer.get(), _size * sizeof(ElementType));
	}
	HeapArray(HeapArray<ElementType>&& other) = default;
	~HeapArray() = default;

	size_t size() const { return _size; }

	iterator begin() { return _buffer.get(); }
	const_iterator const_begin() const { return _buffer.get(); }
	iterator end() { return _buffer.get() + _size; }
	const_iterator const_end() const { return _buffer.get(); }

	ElementType& operator[](const size_t index) const { return _buffer[index]; }
	std::optional<ElementType&> at(const size_t index) const
	{
		if (index < _size)
		{
			return _buffer[index];
		}
		return {};
	}

	HeapArray& operator=(const HeapArray<ElementType>& other) = default;
	HeapArray& operator=(HeapArray<ElementType>&& other) = default;
private:
	size_t _size;
	std::unique_ptr<ElementType[]> _buffer;
};
