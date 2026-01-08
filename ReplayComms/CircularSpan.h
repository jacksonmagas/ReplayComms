#pragma once
#include <span>
#include <ranges>
#include <cstddef>
#include <utility>
#include "ModularArithmetic.h"


/**
* Provides a linearized view of a span interpreted as a circular buffer from index start to index end.
*
* Implemented via two spans which can be concatenated to give the whole range.
* Bulk memory operations should be via the two spans, but iterator/ranges 
* 
* Provides a random access iterator.
* Satisfies ViewableRange for use with ranges library.
*
* If start == end then views the whole span
* If you need a view of none of the span than use fromSize with size 0
*/
template<typename T>
struct CircularSpan {

	CircularSpan(std::span<T> buffer, std::size_t start, size_t end)
	{
		if (start < end) {
			// contiguous segment: [start, end)
			_firstSection = buffer.subspan(start, end - start);
			_secondSection = {};
		}
		else if (start > end) {
			// wrapped segment: [start, size) and [0, end)
			_firstSection = buffer.subspan(start);
			_secondSection = buffer.subspan(0, end);
		}
		else /* start == end */ {
			_firstSection = buffer;
			_secondSection = {};
		}
	};
	CircularSpan(std::span<T> left, std::span<T> right) : _firstSection(left), _secondSection(right) {};
	static CircularSpan fromSize(std::span<T> buffer, std::size_t begin, size_t size) { return CircularSpan(buffer, begin, begin + size); };
	static CircularSpan<const T> fromSizeConst(std::span<const T> buffer, std::size_t begin, size_t size) { return CircularSpan<const T>(buffer, begin, begin + size); };

	std::size_t size() const { return _firstSection.size() + _secondSection.size(); }

	template<typename U>
	class iterator_base {
	public:
		using value_type = U;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::random_access_iterator_tag;
		using pointer = U*;
		using reference = U&;

		iterator_base() {};
		iterator_base(std::span<value_type> left, std::span<value_type> right, size_t index = 0) : _index(index), _first(left), _second(right) {};
		iterator_base(const iterator_base&) = default;
		iterator_base& operator=(const iterator_base&) = default;
		iterator_base(iterator_base&&) = default;
		iterator_base& operator=(iterator_base&&) = default;
		~iterator_base() = default;

		value_type& operator*() const {
			return _index < _first.size()
				? _first[_index]
				: _second[_index - _first.size()];
		}

		value_type& operator[](const difference_type offset) const { return *((*this) + offset); }

		iterator_base& operator++() { ++_index; return *this; }
		iterator_base operator++(int) { iterator_base other = *this; ++(*this); return other; }
		iterator_base& operator--() { --_index; return *this; }
		iterator_base operator--(int) { iterator_base other = *this; --(*this); return other; }

		iterator_base& operator+=(const difference_type increment) { _index += increment; return *this; }
		iterator_base operator+(const difference_type increment) const { iterator_base other = *this; other += increment; return other; }
		friend iterator_base operator+(const difference_type increment, const iterator_base rhs) { iterator_base other = rhs; other += increment; return other; }

		iterator_base& operator-=(const difference_type decrement) { _index -= decrement; return *this; }
		iterator_base operator-(const difference_type decrement) const { iterator_base other = *this; other -= decrement; return other; }
		friend iterator_base operator-(const difference_type decrement, const iterator_base rhs) { iterator_base other = rhs; other -= decrement; return other; }
		friend difference_type operator-(const iterator_base& lhs, const iterator_base& rhs) { return static_cast<difference_type>(lhs._index) - rhs._index; }
		
		auto operator<=>(const iterator_base& other) const {
			return _index <=> other._index;
		}

		bool operator==(const iterator_base& other) const { return _index == other._index; }

	private:
		size_t _index;
		std::span<value_type> _first, _second;
	};

	using iterator = iterator_base<T>;
	using const_iterator = iterator_base<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static_assert(std::random_access_iterator<iterator>);
	static_assert(std::random_access_iterator<const_iterator>);

	iterator begin() { return iterator(_firstSection, _secondSection); }
	iterator end() { return iterator(_firstSection, _secondSection, size()); }

	const_iterator rbegin() const { return const_iterator(_firstSection, _secondSection); }
	const_iterator rend() const { return const_iterator(_firstSection, _secondSection, size()); }

	reverse_iterator rbegin() { return reverse_iterator(end()); }
	reverse_iterator rend() { return reverse_iterator(begin()); }

	const_reverse_iterator begin() const { return const_reverse_iterator(end()); }
	const_reverse_iterator end() const { return const_reverse_iterator(begin()); }

	std::span<T> _firstSection;
	std::span<T> _secondSection;

};

static_assert(std::ranges::viewable_range<CircularSpan<char>>);

template<typename T>
constexpr bool std::ranges::enable_borrowed_range<CircularSpan<T>> = true;

template<typename T>
constexpr bool std::ranges::enable_view<CircularSpan<T>> = true;

