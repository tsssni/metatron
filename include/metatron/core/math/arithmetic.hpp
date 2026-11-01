#pragma once
#include <metatron/core/infra/type.hpp>
#include <metatron/core/math/bit.hpp>
#include <metatron/core/math/cmath.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/stl/concept.hpp>
#include <algorithm>

namespace mtt::core {

template<std::totally_ordered... Ts>
auto constexpr min(Ts... xs) noexcept requires(sizeof...(xs) >= 1) { return std::min({xs...}); }

template<std::totally_ordered... Ts>
auto constexpr max(Ts... xs) noexcept requires(sizeof...(xs) >= 1) { return std::max({xs...}); }

template<typename T>
auto constexpr clamp(T x, T l, T r) noexcept -> T { return std::clamp(x, l, r); }

template<floating_point T>
auto constexpr saturate(T x) noexcept -> T {
    if (!isfinite(x)) return T(0);
    return clamp(x, T(0), T(1));
}

template<typename T>
auto constexpr mod(T x, T y) noexcept -> T requires floating_point<T> || std::integral<T> {
    if constexpr (std::integral<T>) return x % y;
    else return fmod(x, y);
}

template<typename T>
auto constexpr pmod(T x, T y) noexcept -> T {
    return (x % y + y) % y;
}

template<typename T>
auto constexpr guarded_div(T x, T y) noexcept -> T {
    return x / y;
}

template<floating_point T>
auto constexpr guarded_div(T x, T y) noexcept -> T {
    return abs(y) < epsilon<T> ? 0.0 : x / y;
}

template<typename T>
auto constexpr sqr(T x) noexcept -> T requires requires(T x) { {x * x} noexcept -> std::convertible_to<T>; } {
    return x * x;
}

template<std::integral T>
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

template<floating_point T>
auto constexpr pow(T x, usize n) noexcept -> T {
    auto y = T(1);
    for (auto i = 0; i < n; ++i)
        y *= x;
    return y;
}

template<usize n, usize d = 1, typename T>
auto constexpr pow(T x) noexcept -> T requires(d == 1 || floating_point<T>) {
    auto y = [&] {
        if constexpr (d == 1) return x;
        else if constexpr (d == 2) return sqrt(x);
        else if constexpr (d == 3) return cbrt(x);
        else return pow(x, T(1) / T(d));
    }();
    if constexpr (n == 0) return T(1);
    else if constexpr (n == 1) return y;
    else if constexpr (n % 2 == 0) return sqr(pow<n / 2>(y));
    else return sqr(pow<n / 2>(y)) * y;
}

template<floating_point T>
auto constexpr lerp(T x, T y, T alpha) noexcept -> T {
    return (T(1) - alpha) * x + alpha * y;
}

template<typename T>
auto constexpr log2i(T x) noexcept -> usize {
    return std::bit_width(usize(x)) - 1uz;
}

template<typename T>
auto constexpr sign(T x) noexcept -> i32 requires floating_point<T> || std::integral<T> {
    return (x > T(0)) - (x < T(0));
}

}
