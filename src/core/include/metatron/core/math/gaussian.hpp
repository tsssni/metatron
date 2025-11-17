#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/polynomial.hpp>

namespace mtt::math {
    // inverse real error function approximation based on "Approximating the erfinv function" by Mark Giles
    template<typename T>
    requires std::floating_point<T>
    auto erfinv(T x) noexcept -> T {
        auto w = -std::log((T{1} - x) * (T{1} + x));

        auto w1 = w - T{2.5};
        auto w2 = math::sqrt(w) - T{3};

        auto p1 = T(polynomial(w1, Vector<T, 9>{
            1.50140941,     0.246640727,
            -0.00417768164, -0.00125372503,
            0.00021858087, -4.39150654e-06,
            -3.5233877e-06,  3.43273939e-07,
            2.81022636e-08
        }));

        auto p2 = T(polynomial(w2, Vector<T, 9>{
            2.83297682,     1.00167406,
            0.00943887047, -0.0076224613,
            0.00573950773, -0.00367342844,
            0.00134934322,  0.000100950558,
            -0.000200214257
        }));

        return (w < T{5} ? p1 : p2) * x;
    }

    template<typename T>
    requires std::floating_point<T>
    auto gaussian(T x, T mu, T sigma) noexcept -> T {
        return std::exp(-math::sqr(x - mu) / (T{2} * math::sqr(sigma))) / (math::sqrt(T{2} * pi) * sigma);
    }

    template<typename T>
    requires std::floating_point<T>
    auto gaussian_cdf(T x, T mu, T sigma) noexcept -> T {
        return T{0.5} * (T{1} + std::erf((x - mu) / sigma / math::sqrt(T{2})));
    }
}
