#pragma once
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::math {
    template<usize n>
    struct Discrete_Distribution final {
        std::array<f32, n> pdf{0.f};
        std::array<f32, n + 1> cdf{0.f};

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

    struct Array_Distribution final {
        buf<f32> pdf;
        buf<f32> cdf;

        Array_Distribution() noexcept = default;
        Array_Distribution(std::span<f32> ws) noexcept:
        pdf(ws) {
            cdf = ws.size() + 1;
            cdf[0] = 0.f;
            for (auto i = 1; i <= ws.size(); ++i)
                cdf[i] = cdf[i - 1] + pdf[i - 1];
            for (auto i = 0; i < ws.size(); ++i)
                pdf[i] /= cdf[ws.size()];
        }

        auto sample(f32 u) const noexcept -> usize {
            auto i = 0uz;
            for (; i < pdf.size() && u > cdf[i + 1]; ++i);
            return i;
        }
    };
}
