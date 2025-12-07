#pragma once
#include <metatron/core/math/constant.hpp>
#include <cmath>

namespace mtt::math {
    template<typename... Ts>
    requires (std::totally_ordered<Ts> && ...)
    auto constexpr min(Ts... xs) noexcept requires(sizeof...(xs) >= 1) {
        return std::min({xs...});
    }

    template<typename... Ts>
    requires (std::totally_ordered<Ts> && ...)
    auto constexpr max(Ts... xs) noexcept requires(sizeof...(xs) >= 1) {
        return std::max({xs...});
    }

    template<typename T>
    auto constexpr abs(T x) noexcept -> T {
        return std::abs(x);
    }

    template<typename T>
    auto constexpr clamp(T x, T l, T r) noexcept -> T {
        return std::clamp(x, l, r);
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr isnan(T x) noexcept -> bool {
        return std::isnan(x);
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr isinf(T x) noexcept -> bool {
        return std::isinf(x);
    }


    template<typename T>
    requires std::floating_point<T> || std::integral<T>
    auto constexpr mod(T x, T y) noexcept -> T {
        if constexpr (std::integral<T>) return x % y;
        else return std::fmod(x, y);
    }

    template<typename T>
    auto constexpr pmod(T x, T y) noexcept -> T {
        return (x % y + y) % y;
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr guarded_div(T x, T y) noexcept -> T {
        return abs(y) < epsilon<T> ? 0.0 : x / y;
    }

    template<typename T>
    requires std::integral<T>
    auto constexpr pow(T x, T n) noexcept -> T {
        auto y = T{1};
        while (n) {
            if (n & 1)
                y = y * x;
            x = x * x;
            n >>= 1;
        }
        return y;
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr pow(T x, usize n) noexcept -> T {
        auto y = T{1};
        for (auto i = 0; i < n; ++i)
            y *= x;
        return y;
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr sqrt(T x) noexcept -> T {
        return std::sqrt(math::max(T{0}, x));
    }

    template<typename T>
    requires requires(T x) { {x * x} noexcept -> std::convertible_to<T>; }
    auto constexpr sqr(T x) noexcept -> T {
        return x * x;
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr lerp(T x, T y, T alpha) noexcept -> T {
        return (T{1} - alpha) * x + alpha * y;
    }

    template<typename T>
    auto constexpr log2i(T x) noexcept -> usize {
        auto y = usize(x);
        return std::bit_width(y) - 1uz;
    }

    template<typename T>
    requires std::floating_point<T> || std::integral<T>
    auto constexpr sign(T x) noexcept -> i32 {
        return (x > T{0}) - (x < T{0});
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr acos(T x) noexcept -> T {
        return std::acos(math::clamp(x, T{-1}, T{1}));
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr asin(T x) noexcept -> T {
        return std::asin(math::clamp(x, T{-1}, T{1}));
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr atan2(T y, T x) noexcept -> T {
        auto z = std::atan2(y, x);
        if (z < T{0})
            z += T{2} * T{math::pi};
        return z;
    }
}
