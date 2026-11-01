#pragma once
#include <metatron/core/math/cmath.hpp>

namespace mtt::core {

struct Exponential_Distribution final {
    f32 a;
    auto sample(f32 u) const noexcept -> f32;
    auto pdf(f32 x) const noexcept -> f32;
};

auto inline Exponential_Distribution::sample(f32 u) const noexcept -> f32 {
    return -log(1.f - u) / a;
}

auto inline Exponential_Distribution::pdf(f32 x) const noexcept -> f32 {
    return a * exp(-a * x);
}

}
