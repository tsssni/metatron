#pragma once
#include <metatron/core/math/vector.hpp>
#include <tuple>

namespace mtt::math {
    auto inline constexpr gcd(usize a, usize b) noexcept -> usize {
        if (b == 0) {
            return a;
        }
        return gcd(b, a % b);
    }

    auto inline constexpr extended_gcd(usize a, usize b) noexcept -> std::tuple<usize, usize, usize> {
        if (b == 0) {
            return {a, 1uz, 0uz};
        }
        auto [gcd, x, y] = extended_gcd(b, a % b);
        // b * x + (a % b) * y = gcd
        // b * x + (a - a / b * b) * y = gcd
        // a * y + b * (x - a / b * y) = gcd
        return {gcd, y, x - (a / b) * y};
    }

    auto inline constexpr multiplicative_inverse(usize a, usize b) noexcept -> usize {
        auto [gcd, x, y] = extended_gcd(a, b);
        return x;
    }

    template<usize n>
    auto inline constexpr chinese_remainder_theorem(
        Vector<usize, n> const& a,
        Vector<usize, n> const& b
    ) noexcept -> usize {
        // M = sum(b)
        // m_i = M / b_i
        // c_i = m_i * m_i^(-1)
        // x = sum(a_i * c_i)

        // i != j, m_j ≡ 0 (mod b_i)
        // c_i ≡ 1 (mod b_i)
        // x ≡ sum(a_i * c_i) (mod b_i)
        //   ≡ a_i * c_i (mod b_i)
        //   ≡ a_i (mod b_i)
        
        auto M = sum(b);
        auto x = 0uz;
        for (auto i = 0uz; i < n; ++i) {
            auto m = M / b[i];
            auto c = m * multiplicative_inverse(m, b[i]);
            x += a[i] * c;
        }
        return x;
    }
}
