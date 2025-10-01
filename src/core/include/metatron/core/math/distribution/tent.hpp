#pragma once
#include <metatron/core/math/distribution/linear.hpp>
#include <cmath>

namespace mtt::math {
    struct Tent_Distribution final {
        Tent_Distribution(f32 r) noexcept: r(r) {}

        auto sample(f32 u) const noexcept -> f32 {
            if (u < 0.5) {
                return math::lerp(-r, 0.f, Linear_Distribution{0.f, 1.f / r}.sample(u / 0.5f));
            } else {
                return math::lerp(0.f, r, Linear_Distribution{1.f / r, 0.f}.sample((u - 0.5f) / 0.5f));
            }
        }

        auto pdf(f32 x) const noexcept -> f32 {
            return 1.f / r - math::abs(x) / (r * r);
        }

    private:
        f32 r;
    };
}
