#pragma once
#include <metatron/core/distribution/disk.hpp>
#include <metatron/core/math/cmath.hpp>
#include <metatron/core/math/sphere.hpp>

namespace mtt::core {

struct Sphere_Distribution final {
    auto sample(fv2 const& u) const noexcept -> fv3;
    auto pdf() const noexcept -> f32;
};

auto inline Sphere_Distribution::sample(fv2 const& u) const noexcept -> fv3 {
    auto cos_theta = 1.f - 2.f * u[0];
    auto sin_theta = pow<1, 2>(1 - cos_theta * cos_theta);
    auto phi = 2.f * pi * u[1];
    return {sin_theta * cos(phi), cos_theta, sin_theta * sin(phi)};
}

auto inline Sphere_Distribution::pdf() const noexcept -> f32 {
    return 1.f / (4.f * pi);
}

struct Hemisphere_Distribution final {
    auto sample(fv2 const& u) const noexcept -> fv3;

    auto pdf() const noexcept -> f32;
};

auto inline Hemisphere_Distribution::sample(fv2 const& u) const noexcept -> fv3 {
    auto z = u[0];
    auto r = pow<1, 2>(1 - z * z);
    auto phi = 2.f * pi * u[1];
    return {r * cos(phi), r * sin(phi), z};
}

auto inline Hemisphere_Distribution::pdf() const noexcept -> f32 {
    return 1.f / (2.f * pi);
}

struct Cosine_Hemisphere_Distribution final {
    auto sample(fv2 const& u) const noexcept -> fv3;

    auto pdf(f32 cos_theta) const noexcept -> f32;
};

auto inline Cosine_Hemisphere_Distribution::sample(fv2 const& u) const noexcept -> fv3 {
    auto distr = Unifrom_Disk_Distribution{};
    auto d = distr.sample(u);
    return {d[0], pow<1, 2>(1.f - pow<2>(d[0]) - pow<2>(d[1])), d[1]};
}

auto inline Cosine_Hemisphere_Distribution::pdf(f32 cos_theta) const noexcept -> f32 {
    return cos_theta / pi;
}

}
