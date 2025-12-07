#pragma once
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::math {
    template<usize n>
    struct Discrete_Distribution final {
        std::array<f32, n> pdf{0.f};
        std::array<f32, n> cdf{0.f};

        Discrete_Distribution() noexcept = default;
        Discrete_Distribution(std::array<f32, n>&& ws) noexcept:
        pdf(ws) {
            cdf[0] = 0.f;
            for (auto i = 1; i <= n; ++i)
                cdf[i] = cdf[i - 1] + pdf[i - 1];
            for (auto& w: this->pdf)
                w /= cdf.back();
        }

        auto sample(f32 u) const noexcept -> usize {
            auto i = 0uz;
            for (; i < pdf.size() && u > cdf[i + 1]; ++i);
            return i;
        }
    };
}
