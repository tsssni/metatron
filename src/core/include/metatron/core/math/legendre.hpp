#pragma once
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::math {
    template<typename T>
    requires std::floating_point<T>
    auto legendre(i32 l, T x) -> T {
        if (l > 1) {
            auto l0 = T{0}, l1 = x, l2 = T{1};
            auto k0 = T{3}, k1 = T{2}, k2 = T{1};
            for (auto i = 2; i <= l; i++) {
                l0 = (k0 * x * l1 - k2 * l2) / k1;
                l2 = l1; l1 = l0;
                k2 = k1; k1 += T{1}; k0 += T{2};
            }
            return l0;
        } else {
            return l == 0 ? 1 : x;
        }
    }

    template<typename T>
    requires std::floating_point<T>
    auto legendre(i32 l, i32 m, T x) -> T {
        auto mm = T{1};
        if (m > 0) {
            auto s = math::sqrt((T{1} - x) * (T{1} + x));
            auto f = T{1};
            for (auto i = 1; i <= m; i++) {
                mm *= -f * s;
                f += T{2};
            }
        }

        if (l == m) {
            return mm;
        }

        auto mmp1 = x * (T{2} * m + T{1}) * mm;
        if (l == m + 1) {
            return mmp1;
        }

        auto lm2m = mm;
        auto lm1m = mmp1;
        for (auto i = m + 2; i <= l; i++) {
            auto im = (T{0}
            + (T{2} * i - T{1}) * x * lm1m
            - (i + m - T{1}) * lm2m
            ) / (l - m);
            lm2m = lm1m;
            lm1m = im;
        }

        auto lm = lm1m;
        return lm;
    }

    template<typename T>
    requires std::floating_point<T>
    auto legendre_derivative(i32 l, T x) -> std::tuple<T, T> {
        if (l > 1) {
            auto l0 = T{0}, l1 = x, l2 = T{1};
            auto d0 = T{0}, d1 = T{1}, d2 = T{0};
            auto k0 = T{3}, k1 = T{2}, k2 = T{1};
            for (auto i = 2; i <= l; i++) {
                l0 = (k0 * x * l1 - k2 * l2) / k1;
                d0 = d2 + k0 * l1;
                l2 = l1; l1 = l0;
                d2 = d1; d1 = d0;
                k2 = k1; k1 += T{1}; k0 += T{2};
            }
            return {l0, d0};
        } else {
            if (l == 0) {
                return {T{1}, T{0}};
            } else {
                return {T{x}, T{1}};
            }
        }
    }
}
