#pragma once
#include <metatron/core/math/cmath.hpp>
#include <metatron/core/stl/concept.hpp>

namespace mtt::core {

template<typename T, size_t n>
auto estrin(T const& x, vector<T, n> const& c) -> T {
    auto constexpr n_rec = (n - 1) / 2, n_fma = n / 2;

    auto c_rec = vector<T, n_rec + 1>{};
    for (auto i = 0; i < n_fma; ++i)
        c_rec[i] = fma(x, c[2 * i + 1], c[2 * i]);

    if constexpr (n_rec == n_fma) c_rec[n_rec] = c[n - 1];
    if constexpr (n_rec == 0) return c_rec[0];
    else return estrin(pow<2>(x), c_rec);
}

template<floating_point T, usize n>
auto constexpr polynomial(T x, vector<T, n> const& c) noexcept -> T {
    return estrin(x, c);
}

}
