#pragma once
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::math {
    template<typename T>
    struct Complex {
        T r = T(0);
        T i = T(0);

        auto constexpr operator+(cref<Complex> rhs) const noexcept -> Complex {
            return {r + rhs.r, i + rhs.i};
        }

        auto constexpr operator+=(cref<Complex> rhs) noexcept -> ref<Complex> {
            *this = *this + rhs;
            return *this;
        }

        auto constexpr operator+(cref<T> rhs) const noexcept -> Complex {
            return *this + Complex{rhs};
        }

        auto constexpr operator-(cref<Complex> rhs) const noexcept -> Complex {
            return {r - rhs.r, i - rhs.i};
        }

        auto constexpr operator-=(cref<Complex> rhs) noexcept -> ref<Complex> {
            *this = *this - rhs;
            return *this;
        }

        auto constexpr operator-(cref<T> rhs) const noexcept -> Complex {
            return *this - Complex{rhs};
        }

        auto constexpr operator-() const noexcept -> Complex {
            return {-r, -i};
        }

        auto constexpr operator*(cref<Complex> rhs) const noexcept -> Complex {
            return {
                r * rhs.r - i * rhs.i,
                r * rhs.i + i * rhs.r,
            };
        }

        auto constexpr operator*=(cref<Complex> rhs) noexcept -> ref<Complex> {
            *this = *this * rhs;
            return *this;
        }

        auto constexpr operator*(cref<T> rhs) const noexcept -> Complex {
            return *this * Complex{rhs};
        }

        auto constexpr operator/(cref<Complex> rhs) const noexcept -> Complex {
            auto denom = rhs.r * rhs.r + rhs.i * rhs.i;
            return {
                math::guarded_div(r * rhs.r + i * rhs.i, denom),
                math::guarded_div(i * rhs.r - r * rhs.i, denom),
            };
        }

        auto constexpr operator/=(cref<Complex> rhs) noexcept -> ref<Complex> {
            *this = *this / rhs;
            return *this;
        }

        auto constexpr operator/(cref<T> rhs) const noexcept -> Complex {
            return *this / Complex{rhs};
        }

        template<usize idx>
        auto constexpr get() const noexcept -> cref<T> {
            static_assert(idx < 2, "index out of bounds");
            if constexpr (idx == 0) return r;
            else return i;
        }

        template<usize idx>
        auto constexpr get() noexcept -> ref<T> {
            static_assert(idx < 2, "index out of bounds");
            if constexpr (idx == 0) return r;
            else return i;
        }
    };

    template<typename C, typename T>
    concept complex = std::is_same_v<C, Complex<T>>;

    template<typename T>
    auto constexpr operator+(cref<T> lhs, cref<Complex<T>> rhs) noexcept -> Complex<T> {
        return rhs + lhs;
    }

    template<typename T>
    auto constexpr operator-(cref<T> lhs, cref<Complex<T>> rhs) noexcept -> Complex<T> {
        return -rhs + lhs;
    }

    template<typename T>
    auto constexpr operator*(cref<T> lhs, cref<Complex<T>> rhs) noexcept -> Complex<T> {
        return rhs * lhs;
    }

    template<typename T>
    auto constexpr operator/(cref<T> lhs, cref<Complex<T>> rhs) noexcept -> Complex<T> {
        return Complex<T>{lhs} / rhs;
    }

    template<usize idx, typename T>
    auto constexpr get(cref<Complex<T>> z)  noexcept -> cref<T> {
        return z.template get<idx>();
    }

    template<usize idx, typename T>
    auto constexpr get(Complex<T>& z) noexcept -> ref<T> {
        return z.template get<idx>();
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr norm(cref<Complex<T>> z) noexcept -> T {
        return z.r * z.r + z.i * z.i;
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr abs(cref<Complex<T>> z) noexcept -> T {
        return math::pow<1,2>(norm(z));
    }

    template<typename T>
    requires std::floating_point<T>
    auto constexpr sqrt(cref<Complex<T>> z) noexcept -> Complex<T> {
        // z = a + bi, w = u + vi = sqrt(z)
        // u^2 - v^2 = a, 2uv = b
        auto [a, b] = z;

        if (b != T(0)) {
            // v = b / 2u
            // u^4 - a u^2 - b^2 / 4 = 0
            // u^2 = (a + sqrt(a^2 + b^2)) / 2
            auto u = math::pow<1,2>((a + abs(z)) * T(0.5));
            auto v = T(0.5) * b / u;
            return {u, v};
        } else if (a >= T(0)) {
            return {math::pow<1,2>(a), T(0)};
        } else {
            return {T(0), math::pow<1,2>(-a)};
        }
    }

    template <typename T>
    requires std::floating_point<T>
    auto constexpr sqr(cref<Complex<T>> z) noexcept -> Complex<T> {
        return z * z;
    }

    template<usize n, usize d = 1, typename T>
    requires std::floating_point<T>
    auto constexpr pow(Complex<T> x) noexcept -> Complex<T> {
        auto y = [&] {
            if constexpr (d == 1) return x;
            else if constexpr (d == 2) return sqrt(x);
        }();
        if constexpr (n == 0) return Complex<T>{T(1), T(0)};
        else if constexpr (n == 1) return y;
        else if constexpr (n % 2 == 0) { auto z = pow<n/2>(y); return z * z; }
        else { auto z = pow<n/2>(y); return z * z * y; }
    }

    template<typename T>
    auto inline constexpr guarded_div(cref<Complex<T>> x, cref<Complex<T>> y) noexcept -> Complex<T> {
        return x / y;
    }

    template<typename T>
    auto inline constexpr guarded_div(T x, cref<Complex<T>> y) noexcept -> Complex<T> {
        return x / y;
    }

    template<typename T>
    auto inline constexpr guarded_div(cref<Complex<T>> x, T y) noexcept -> Complex<T> {
        return x / y;
    }
}

namespace mtt {
    #define MTT_COMPLEX_ALIAS(p, T)\
    using p##c = math::Complex<T>;

    MTT_COMPLEX_ALIAS(f, f32)
    MTT_COMPLEX_ALIAS(d, f64)
}
