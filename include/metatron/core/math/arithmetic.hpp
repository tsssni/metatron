#pragma once
#include <metatron/core/stl/concept.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/bit.hpp>
#include <algorithm>
#include <metatron/core/stl/cmath.hpp>

namespace mtt::math {
    template<typename... Ts>
    requires (stl::totally_ordered<Ts> && ...)
    auto constexpr min(Ts... xs) noexcept requires(sizeof...(xs) >= 1) {
        return std::min({xs...});
    }

    template<typename... Ts>
    requires (stl::totally_ordered<Ts> && ...)
    auto constexpr max(Ts... xs) noexcept requires(sizeof...(xs) >= 1) {
        return std::max({xs...});
    }

    template<typename T>
    auto constexpr abs(T x) noexcept -> T {
        return stl::abs(x);
    }

    template<typename T>
    auto constexpr clamp(T x, T l, T r) noexcept -> T {
        return std::clamp(x, l, r);
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr isnan(T x) noexcept -> bool {
        return stl::isnan(x);
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr isinf(T x) noexcept -> bool {
        return stl::isinf(x);
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr isfinite(T x) noexcept -> bool {
        return stl::isfinite(x);
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr saturate(T x) noexcept -> T {
        if (!isfinite(x)) return T(0);
        return clamp(x, T(0), T(1));
    }

    template<typename T>
    requires stl::floating_point<T> || stl::integral<T>
    auto constexpr mod(T x, T y) noexcept -> T {
        if constexpr (stl::integral<T>) return x % y;
        else return stl::fmod(x, y);
    }

    template<typename T>
    auto constexpr pmod(T x, T y) noexcept -> T {
        return (x % y + y) % y;
    }

    template<typename T>
    auto constexpr guarded_div(T x, T y) noexcept -> T {
        return x / y;
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr guarded_div(T x, T y) noexcept -> T {
        return abs(y) < epsilon<T> ? 0.0 : x / y;
    }

    template<typename T>
    requires stl::integral<T>
    auto constexpr pow(T x, T n) noexcept -> T {
        auto y = T(1);
        while (n) {
            if (n & 1)
                y = y * x;
            x = x * x;
            n >>= 1;
        }
        return y;
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr pow(T x, usize n) noexcept -> T {
        auto y = T(1);
        for (auto i = 0; i < n; ++i)
            y *= x;
        return y;
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr sqrt(T x) noexcept -> T {
        return stl::sqrt(math::max(T(0), x));
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr cbrt(T x) noexcept -> T {
        return stl::cbrt(x);
    }

    template<usize n, usize d = 1, typename T>
    requires (d == 1 || stl::floating_point<T>)
    auto constexpr pow(T x) noexcept -> T {
        auto y = [&] {
            if constexpr (d == 1) return x;
            else if constexpr (d == 2) return sqrt(x);
            else if constexpr (d == 3) return cbrt(x);
            else return stl::pow(x, T(1) / T(d));
        }();
        if constexpr (n == 0) return T(1);
        else if constexpr (n == 1) return y;
        else if constexpr (n % 2 == 0) { auto z = pow<n/2>(y); return z * z; }
        else { auto z = pow<n/2>(y); return z * z * y; }
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr pow(T x, T n) noexcept -> T {
        return stl::pow(x, n);
    }

    template<typename T>
    requires requires(T x) { {x * x} noexcept -> stl::convertible_to<T>; }
    auto constexpr sqr(T x) noexcept -> T {
        return x * x;
    }

    template<typename T>
    auto constexpr exp(T x) noexcept -> T {
        return stl::exp(x);
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr lerp(T x, T y, T alpha) noexcept -> T {
        return (T(1) - alpha) * x + alpha * y;
    }

    template<typename T>
    auto constexpr log2i(T x) noexcept -> usize {
        return std::bit_width(usize(x)) - 1uz;
    }

    template<typename T>
    requires stl::floating_point<T> || stl::integral<T>
    auto constexpr sign(T x) noexcept -> i32 {
        return (x > T(0)) - (x < T(0));
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr acos(T x) noexcept -> T {
        return stl::acos(math::clamp(x, T(-1), T(1)));
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr asin(T x) noexcept -> T {
        return stl::asin(math::clamp(x, T(-1), T(1)));
    }

    template<typename T>
    requires stl::floating_point<T>
    auto constexpr atan2(T y, T x) noexcept -> T {
        auto z = stl::atan2(y, x);
        if (z < T(0))
            z += T(2) * T(math::pi);
        return z;
    }
}
