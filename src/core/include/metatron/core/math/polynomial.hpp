#pragma once
#include <metatron/core/math/vector.hpp>

namespace mtt::math {
    template <typename T, size_t n>
    auto estrin(cref<T> x, cref<Vector<T, n>> c) -> T {
        auto constexpr n_rec = (n - 1) / 2, n_fma = n / 2;

        auto c_rec = Vector<T, n_rec + 1>{};
        for (size_t i = 0; i < n_fma; ++i)
            c_rec[i] = std::fma(x, c[2 * i + 1], c[2 * i]);

        if constexpr (n_rec == n_fma) c_rec[n_rec] = c[n - 1];
        if constexpr (n_rec == 0) return c_rec[0];
        else return estrin(math::sqr(x), c_rec);
    }

    template<typename T, usize n>
    requires std::floating_point<T>
    auto constexpr polynomial(T x, cref<Vector<T, n>> c) noexcept -> T {
        return estrin(x, c);
    }
}
