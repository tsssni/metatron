#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <cmath>

namespace mtt::math {
    auto inline sinc(f32 x) noexcept -> f32 {
        return guarded_div(std::sin(pi * x), pi * x);
    };

    auto inline windowed_sinc(f32 x, f32 tau) noexcept -> f32 {
        return sinc(x) * sinc(x / tau);
    }

    auto inline radians(f32 degree) -> f32 {
        return degree / 180.f * math::pi;
    }

    auto inline degrees(f32 radian) -> f32 {
        return radian / math::pi * 180.f;
    }
}
