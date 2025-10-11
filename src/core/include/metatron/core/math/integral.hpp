#pragma once
#include <metatron/core/math/legendre.hpp>
#include <metatron/core/stl/print.hpp>
#include <vector>

namespace mtt::math {
    template<typename T>
    requires std::floating_point<T>
    auto gauss_legendre(i32 n) {
        using A = std::vector<T>;
        auto p = A(n), w = A(n);
        n--;

        if (n == 0) {
            p[0] = T{0}; w[0] = T{2};
        } else {
            p[0] = -math::sqrt(T{1.0 / 3.0});
            p[1] = -p[0];
            w[0] =  w[1] = T{1};
        }

        for (auto i = 0; i <= (n + 1) / 2; ++i) {
            auto x = -std::cos(f64(2 * i + 1) / f64(2 * n + 2) * math::piv<f64>);
            auto it = 0;

            while (true) {
                if (it > 20) {
                    std::println("gauss-legendre({}) not converge", n);
                    std::abort();
                }

                auto [y, d] = legendre_derivative(n + 1, x);
                auto step = y / d;
                x -= step;

                if (math::abs(step) <= 4.0 * std::abs(x) * math::epsilon<f64>) {
                    break;
                }
                ++it;
            }

            auto [y, d] = legendre_derivative(n + 1, x);
            w[i] = w[n - i] = T(2.0 / ((1.0 - x * x) * (d * d)));
            p[i] = x, p[n - i] = -x;
        }

        if ((n % 2) == 0) {
            auto [y, d] = legendre_derivative(n + 1, 0.0);
            w[n / 2] = T(2.0 / (d * d));
            p[n / 2] = T{0};
        }

        return std::tuple<A, A>{p, w};
    }
}

