#pragma once
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/concept.hpp>

namespace mtt::core {

template<typename T>
struct complex {
    T r = T(0);
    T i = T(0);

    auto constexpr operator+(complex const& rhs) const noexcept -> complex;
    auto constexpr operator+=(complex const& rhs) noexcept -> complex&;
    auto constexpr operator+(T const& rhs) const noexcept -> complex;

    auto constexpr operator-(complex const& rhs) const noexcept -> complex;
    auto constexpr operator-=(complex const& rhs) noexcept -> complex&;
    auto constexpr operator-(T const& rhs) const noexcept -> complex;
    auto constexpr operator-() const noexcept -> complex;

    auto constexpr operator*(complex const& rhs) const noexcept -> complex;
    auto constexpr operator*=(complex const& rhs) noexcept -> complex&;
    auto constexpr operator*(T const& rhs) const noexcept -> complex;

    auto constexpr operator/(complex const& rhs) const noexcept -> complex;
    auto constexpr operator/=(complex const& rhs) noexcept -> complex&;
    auto constexpr operator/(T const& rhs) const noexcept -> complex;

    template<usize idx>
    auto constexpr get() const noexcept -> T const& requires(idx < 2);
    template<usize idx>
    auto constexpr get() noexcept -> T& requires(idx < 2);
};

template<typename T>
auto constexpr complex<T>::operator+(complex const& rhs) const noexcept -> complex { return {r + rhs.r, i + rhs.i}; }
template<typename T>
auto constexpr complex<T>::operator+=(complex const& rhs) noexcept -> complex& { return *this = *this + rhs; }
template<typename T>
auto constexpr complex<T>::operator+(T const& rhs) const noexcept -> complex { return *this + complex{rhs}; }

template<typename T>
auto constexpr complex<T>::operator-(complex const& rhs) const noexcept -> complex { return {r - rhs.r, i - rhs.i}; }
template<typename T>
auto constexpr complex<T>::operator-=(complex const& rhs) noexcept -> complex& { return *this = *this - rhs; }
template<typename T>
auto constexpr complex<T>::operator-(T const& rhs) const noexcept -> complex { return *this - complex{rhs}; }
template<typename T>
auto constexpr complex<T>::operator-() const noexcept -> complex { return {-r, -i}; }

template<typename T>
auto constexpr complex<T>::operator*(complex const& rhs) const noexcept -> complex {
    return {
        r * rhs.r - i * rhs.i,
        r * rhs.i + i * rhs.r,
    };
}

template<typename T>
auto constexpr complex<T>::operator*=(complex const& rhs) noexcept -> complex& { return *this = *this * rhs; }
template<typename T>
auto constexpr complex<T>::operator*(T const& rhs) const noexcept -> complex { return *this * complex{rhs}; }

template<typename T>
auto constexpr complex<T>::operator/(complex const& rhs) const noexcept -> complex {
    auto denom = rhs.r * rhs.r + rhs.i * rhs.i;
    return {
        (r * rhs.r + i * rhs.i) / denom,
        (i * rhs.r - r * rhs.i) / denom,
    };
}

template<typename T>
auto constexpr complex<T>::operator/=(complex const& rhs) noexcept -> complex& { return *this = *this / rhs; }
template<typename T>
auto constexpr complex<T>::operator/(T const& rhs) const noexcept -> complex { return *this / complex{rhs}; }

template<typename T>
template<usize idx>
auto constexpr complex<T>::get() const noexcept -> T const& requires(idx < 2) { return idx == 0 ? r : i; }
template<typename T>
template<usize idx>
auto constexpr complex<T>::get() noexcept -> T& requires(idx < 2) { return idx == 0 ? r : i; }

template<typename T>
auto constexpr operator+(T const& lhs, complex<T> const& rhs) noexcept -> complex<T> { return rhs + lhs; }
template<typename T>
auto constexpr operator-(T const& lhs, complex<T> const& rhs) noexcept -> complex<T> { return -rhs + lhs; }
template<typename T>
auto constexpr operator*(T const& lhs, complex<T> const& rhs) noexcept -> complex<T> { return rhs * lhs; }
template<typename T>
auto constexpr operator/(T const& lhs, complex<T> const& rhs) noexcept -> complex<T> { return complex<T>{lhs} / rhs; }

template<usize idx, typename T>
auto constexpr get(complex<T> const& z) noexcept -> T const& requires(idx < 2) { return z.template get<idx>(); }
template<usize idx, typename T>
auto constexpr get(complex<T>& z) noexcept -> T& requires(idx < 2) { return z.template get<idx>(); }

template<floating_point T>
auto constexpr norm(complex<T> const& z) noexcept -> T { return z.r * z.r + z.i * z.i; }
template<floating_point T>
auto constexpr abs(complex<T> const& z) noexcept -> T { return pow<1, 2>(norm(z)); }

template<floating_point T>
auto constexpr sqrt(complex<T> const& z) noexcept -> complex<T> {
    // z = a + bi, w = u + vi = sqrt(z)
    // u^2 - v^2 = a, 2uv = b
    auto [a, b] = z;

    if (b != T(0)) {
        // v = b / 2u
        // u^4 - a u^2 - b^2 / 4 = 0
        // u^2 = (a + sqrt(a^2 + b^2)) / 2
        auto u = pow<1, 2>((a + abs(z)) * T(0.5));
        auto v = T(0.5) * b / u;
        return {u, v};
    } else if (a >= T(0)) {
        return {pow<1, 2>(a), T(0)};
    } else {
        return {T(0), pow<1, 2>(-a)};
    }
}

template<floating_point T>
auto constexpr sqr(complex<T> const& z) noexcept -> complex<T> { return z * z; }

template<usize n, usize d = 1, floating_point T>
auto constexpr pow(complex<T> x) noexcept -> complex<T> {
    auto y = [&] {
        if constexpr (d == 1) return x;
        else if constexpr (d == 2) return sqrt(x);
    }();
    if constexpr (n == 0) return {T(1), T(0)};
    else if constexpr (n == 1) return y;
    else if constexpr (n % 2 == 0) return sqr(pow<n / 2>(y));
    else return sqr(pow<n / 2>(y)) * y;
}

template<typename T>
auto inline constexpr guarded_div(complex<T> const& x, complex<T> const& y) noexcept -> complex<T> { return x / y; }
template<typename T>
auto inline constexpr guarded_div(T x, complex<T> const& y) noexcept -> complex<T> { return x / y; }
template<typename T>
auto inline constexpr guarded_div(complex<T> const& x, T y) noexcept -> complex<T> { return x / y; }

}

namespace mtt {

#define MTT_COMPLEX_ALIAS(p, T) using p##c = core::complex<T>;
MTT_COMPLEX_ALIAS(h, f16)
MTT_COMPLEX_ALIAS(f, f32)
MTT_COMPLEX_ALIAS(d, f64)
#undef MTT_COMPLEX_ALIAS

}
