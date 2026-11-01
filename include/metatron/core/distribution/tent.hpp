#pragma once
#include <metatron/core/distribution/linear.hpp>

namespace mtt::core {

struct Tent_Distribution final {
    f32 r;
    auto sample(f32 u) const noexcept -> f32;
    auto pdf(f32 x) const noexcept -> f32;
};

auto inline Tent_Distribution::sample(f32 u) const noexcept -> f32 {
    return u < 0.5
        ? lerp(-r, 0.f, Linear_Distribution{0.f, 1.f / r}.sample(u / 0.5f))
        : lerp(0.f, r, Linear_Distribution{1.f / r, 0.f}.sample((u - 0.5f) / 0.5f));
}

auto inline Tent_Distribution::pdf(f32 x) const noexcept -> f32 {
    return 1.f / r - abs(x) / (r * r);
}

}
