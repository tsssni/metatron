#pragma once
#include <limits>

namespace metatron::math {
	auto constexpr pi = 3.14159265358979f;
	auto constexpr e = 2.7182881828458f;

	template<typename T>
	auto constexpr epsilon = std::numeric_limits<T>::epsilon();
	template<typename T>
	auto constexpr minv = std::numeric_limits<T>::min();
	template<typename T>
	auto constexpr maxv = std::numeric_limits<T>::max();
	template<typename T>
	auto constexpr high = std::numeric_limits<T>::max();
	template<typename T>
	auto constexpr low = std::numeric_limits<T>::lowest();
	template<typename T>
	auto constexpr inf = std::numeric_limits<T>::infinity();
}
