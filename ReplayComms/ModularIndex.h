#pragma once
#include "ModularArithmetic.h"
#include "InvariantGuard.h"
#include <array>

/**
* An index for a fixed size container which exhibits "wrapping" behavior.
* 
* Does not protect from overflow if the index exceeds the maximum value of size_t;
*/
template<typename T = size_t>
class ModularIndex
{
public:
	static auto invariants() {
		using Inv = Invariant<ModularIndex>;
		return std::to_array({
			Inv{"index < size", [](const ModularIndex& t) { return t._index < t._size;}},
		});
	}

	using value_type = T;
	using difference_type = std::conditional_t<
        std::is_integral_v<value_type>,
        std::make_signed_t<value_type>,
        decltype(std::declval<value_type>() - std::declval<value_type>())
    >;

	constexpr ModularIndex(const size_t size, const size_t index) : _index(index % size), _size(size) {
		InvariantGuard g(this);
	};

	ModularIndex& operator=(const size_t other) { _index = other % _size; }
	constexpr operator value_type() const { InvariantGuard g(this); return _index; }
	ModularIndex& operator++() { InvariantGuard g(this);  _index = ModularArithmetic::add<difference_type>(_index, 1, _size); return *this; }
	ModularIndex& operator--() { InvariantGuard g(this);  _index = ModularArithmetic::subtract<difference_type>(_index, 1, _size); return *this; } // avoid underflow when index is 0
	ModularIndex operator++(int) { InvariantGuard g(this);  ModularIndex prev = *this; ++(*this);  return prev; }
	ModularIndex operator--(int) { InvariantGuard g(this);  ModularIndex prev = *this; --(*this);  return prev; }
	ModularIndex& operator+=(const difference_type increment) { InvariantGuard g(this);  _index = ModularArithmetic::add<difference_type>(_index, increment, _size); return *this; }
	constexpr friend ModularIndex operator+(ModularIndex lhs, difference_type rhs) { InvariantGuard g(this);  lhs += rhs; return lhs; };
	ModularIndex& operator-=(const difference_type decrement) { InvariantGuard g(this);  _index = ModularArithmetic::subtract<difference_type>(_index, decrement, _size); return *this; } // avoid underflow
	constexpr friend ModularIndex operator-(ModularIndex lhs, difference_type rhs) { InvariantGuard g(this);  lhs -= rhs; return lhs; };

	// Illegal to compare the distance of indices of different modulus
	constexpr friend difference_type operator-(const ModularIndex& lhs, const ModularIndex& rhs) { InvariantGuard g(this);  return ModularArithmetic::subtract<difference_type>(lhs._index, rhs._index, lhs._size); }

	/*
	* Operator== falls back on the size_t conversion, this function allows checking if two indicies are identical.
	*/
	constexpr bool sameAs(const ModularIndex& other) const { InvariantGuard g(this);  return _index == other._index && _size == other._size;  }

	const size_t _size;
private:
	value_type _index;
};


template<typename T = size_t>
class AtomicModularIndex
{
public:
	using value_type = T;
	using difference_type = std::conditional_t<
        std::is_integral_v<value_type>,
        std::make_signed_t<value_type>,
        decltype(std::declval<value_type>() - std::declval<value_type>())
    >;

	AtomicModularIndex(ModularIndex<T> original) : _size(original._size), _index(original % size) {
	};

	ModularIndex<value_type> load() const { return { _index.load(), _size }; }
	void store(ModularIndex<T> newVal) const { _index = newVal; }
	void wait(T expectedOldVal, std::memory_order order = std::memory_order::seq_cst) { _index.wait(expectedOldVal, order); }
	inline void notify_all() volatile noexcept { _index.notify_all(); }

	static constexpr bool is_always_lock_free = std::atomic<value_type>::is_always_lock_free;

	const size_t _size;
private:
	std::atomic<value_type> _index;
};
