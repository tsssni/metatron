#pragma once
#include <metatron/core/math/sphere.hpp>

namespace mtt::core {

struct Cone_Distribution final {
    f32 cos_theta_max;
    auto sample(fv2 const& u) const noexcept -> fv3;
    auto pdf() const noexcept -> f32;
};

auto inline Cone_Distribution::sample(fv2 const& u) const noexcept -> fv3 {
    auto cos_theta = 1.f - u[0] * (1.f - cos_theta_max);
    auto phi = u[1] * 2.f * pi;
    return unit_spherical_to_cartesian(cos_theta, phi);
}

auto inline Cone_Distribution::pdf() const noexcept -> f32 {
    return 1.f / (2.f * pi * (1.f - cos_theta_max));
}

}
