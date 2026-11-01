#pragma once
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/cmath.hpp>

namespace mtt::core {

auto constexpr sinc(f32 x) noexcept -> f32 {
    if (abs(x) < epsilon<f32>) return 1.f;
    return sin(pi * x) / (pi * x);
};

auto constexpr windowed_sinc(f32 x, f32 tau) noexcept -> f32 {
    return sinc(x) * sinc(x / tau);
}

auto constexpr radians(f32 degree) noexcept -> f32 {
    return degree / 180.f * pi;
}

auto constexpr degrees(f32 radian) noexcept -> f32 {
    return radian / pi * 180.f;
}

}
