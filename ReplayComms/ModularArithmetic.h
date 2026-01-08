#pragma once

namespace ModularArithmetic {
	template<typename T, typename U>
	constexpr U add(const T first, const T second, const U mod) {
		U leftPositive = ((first % static_cast<T>(mod)) + mod);
		U rightPositive = ((second % static_cast<T>(mod)) + mod);
		return (leftPositive + rightPositive) % mod;
	}

	template<typename T, typename U>
	constexpr U subtract(const T first, const T second, const U mod) {
		U leftPositive = (first % static_cast<T>(mod)) + (2 * mod);
		U rightPositive = (second % static_cast<T>(mod)) + mod;
		return (leftPositive - rightPositive) % mod;
	}
}
