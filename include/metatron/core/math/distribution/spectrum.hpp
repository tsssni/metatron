#pragma once
#include <metatron/core/stl/cmath.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::math {
    struct Spectrum_Distribution final {
        auto sample(f32 u) const noexcept -> f32 {
            return 538.f - 138.888889f * stl::atanh(0.85691062f - 1.82750197f * u);
        }

        auto pdf(f32 x) const noexcept -> f32 {
            if (x < 360.f || x > 830.f) return 0.f;
            return 0.0039398042f / math::pow<2>(stl::cosh(0.0072f * (x - 538.f)));
        }
    };
}
