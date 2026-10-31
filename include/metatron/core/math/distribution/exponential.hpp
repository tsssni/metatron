#pragma once
#include <metatron/core/stl/cmath.hpp>

namespace mtt::math {
    struct Exponential_Distribution final {
        Exponential_Distribution(f32 a) noexcept: a(a) {}

        auto sample(f32 u) const noexcept -> f32 {
            return -stl::log(1.f - u) / a;
        }

        auto pdf(f32 x) const noexcept -> f32 {
            return a * exp(-a * x);
        }

    private:
        f32 a;
    };
}
