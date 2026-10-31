#pragma once
#include <metatron/core/stl/cmath.hpp>
#include <metatron/core/stl/concept.hpp>

namespace mtt::math {
    template <typename T, size_t n>
    auto estrin(T const& x, vector<T, n> const& c) -> T {
        auto constexpr n_rec = (n - 1) / 2, n_fma = n / 2;

        auto c_rec = vector<T, n_rec + 1>{};
        for (auto i = 0; i < n_fma; ++i)
            c_rec[i] = stl::fma(x, c[2 * i + 1], c[2 * i]);

        if constexpr (n_rec == n_fma) c_rec[n_rec] = c[n - 1];
        if constexpr (n_rec == 0) return c_rec[0];
        else return estrin(math::pow<2>(x), c_rec);
    }

    template<typename T, usize n>
    requires stl::floating_point<T>
    auto constexpr polynomial(T x, vector<T, n> const& c) noexcept -> T {
        return estrin(x, c);
    }
}
