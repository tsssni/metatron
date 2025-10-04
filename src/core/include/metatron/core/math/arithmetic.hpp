#pragma once
#include <metatron/core/math/constant.hpp>
#include <cmath>

namespace mtt::math {
    template<typename T>
    auto inline constexpr abs(T x) noexcept -> T {
        return std::abs(x);
    }

    template<typename T>
    auto inline constexpr pmod(T x, T y) noexcept -> T {
        return (x % y + y) % y;
    }

    template<typename T>
    requires std::floating_point<T>
    auto inline constexpr guarded_div(T x, T y) noexcept -> T {
        return abs(y) < epsilon<T> ? 0.0 : x / y;
    }

    template<typename T>
    requires std::integral<T>
    auto inline constexpr pow(T x, T n) noexcept -> T {
        auto y = T{1};
        while (n) {
            if (n & 1) {
                y = y * x;
            }
            x = x * x;
            n >>= 1;
        }
        return y;
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr pow(T x, usize n) noexcept -> T {
        auto y = T{1};
        for (auto i = 0; i < n; i++) {
            y *= x;
        }
        return y;
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr sqrt(T x) noexcept -> T {
        return std::sqrt(std::max(T{0}, x));
    }

    template<typename T>
    requires requires(T x) { {x * x} noexcept -> std::convertible_to<T>; }
    auto constexpr sqr(T x) noexcept -> T {
        return x * x;
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr lerp(T x, T y, T alpha) noexcept -> T {
        return (T{1.0} - alpha) * x + alpha * y;
    }

    template<typename T>
    auto constexpr log2i(T x) noexcept -> usize {
        auto y = usize(x);
        return std::bit_width(y) - 1uz;
    }
}
