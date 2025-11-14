#pragma once
#include <vector>

namespace mtt::math {
    struct Discrete_Distribution final {
        std::vector<f32> pdf;
        std::vector<f32> cdf;

        Discrete_Distribution() noexcept = default;
        Discrete_Distribution(cref<std::vector<f32>> ws) noexcept
        : pdf(ws) {
            cdf.push_back(0.f);
            for (auto w: this->pdf)
                cdf.push_back(cdf.back() + w);
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
