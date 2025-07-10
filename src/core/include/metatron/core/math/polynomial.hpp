#pragma once
#include <array>
#include <cmath>

namespace mtt::math {
	template<usize n>
	struct Polynomial final {
		Polynomial(std::array<f32, n + 1>&& c): c(c) {}
		auto operator()(f32 x) const noexcept -> f32 {
			auto y = 0.f;
			for (auto i = 0; i <= n; i++) {
				y += c[i] * std::powf(x, i);
			}
			return y;
		}
	private:
		std::array<f32, n + 1> c;
	};
}
