#pragma once
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::math {
    struct Spectrum_Distribution final {
        auto sample(f32 u) const noexcept -> f32 {
            return 538.f - 138.888889f * std::atanh(0.85691062f - 1.82750197f * u);
        }

        auto pdf(f32 x) const noexcept -> f32 {
            if (x < 360.f || x > 830.f) return 0.f;
            return 0.0039398042f / math::sqr(std::cosh(0.0072f * (x - 538.f)));
        }

    private:
        f32 r;
    };
}
