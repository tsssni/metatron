#pragma once
#include <array>
#include <cmath>

namespace mtt::math {
	template<typename T, usize n>
	requires std::floating_point<T>
	auto constexpr polynomial(T x, std::array<T, n> c) noexcept -> T {
		auto y = 0.f;
		for (auto i = 0; i < n; i++) {
			y += c[i] * std::pow(x, i);
		}
		return y;
	}
}
