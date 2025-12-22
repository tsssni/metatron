#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <cmath>

namespace mtt::math {
    auto constexpr sinc(f32 x) noexcept -> f32 {
        return guarded_div(std::sin(pi * x), pi * x);
    };

    auto constexpr windowed_sinc(f32 x, f32 tau) noexcept -> f32 {
        return sinc(x) * sinc(x / tau);
    }

    auto constexpr radians(f32 degree) noexcept -> f32 {
        return degree / 180.f * math::pi;
    }

    auto constexpr degrees(f32 radian) noexcept -> f32 {
        return radian / math::pi * 180.f;
    }
}
