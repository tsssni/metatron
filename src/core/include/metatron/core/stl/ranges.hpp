#pragma once
#include <ranges>

namespace mtt::stl::ranges::views {
#ifdef __APPLE__
	auto inline constexpr cartesian_product = [](auto a, auto b) {
		return a | std::views::transform([&](auto i) {
			return b | std::views::transform([i](auto j) {
				return std::make_pair(i, j);
			});
		}) | std::views::join;
	};
#else
	auto inline constexpr cartesian_product = std::views::cartesian_product;
#endif
}

namespace mtt::stl {
	namespace views = ranges::views;
}
