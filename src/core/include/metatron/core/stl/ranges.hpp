#pragma once
#include <ranges>

namespace mtt::stl::ranges::views {
    auto inline constexpr cartesian_product = [](auto&& a, auto&& b) {
        return a | std::views::transform([b = std::forward<decltype(b)>(b)](auto&& i) mutable {
            return b | std::views::transform([i = std::forward<decltype(i)>(i)](auto&& j) {
                return std::make_pair(i, j);
            });
        }) | std::views::join;
    };
}

namespace mtt::stl {
    namespace views = ranges::views;
}
