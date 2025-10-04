#pragma once
#include <metatron/core/math/distribution/disk.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/sphere.hpp>

namespace mtt::math {
    struct Sphere_Distribution final {
        Sphere_Distribution() noexcept = default;

        auto sample(Vector<f32, 2> const& u) const noexcept -> Vector<f32, 3> {
            auto cos_theta = 1.f - 2.f * u[0];
            auto sin_theta = math::sqrt(1 - cos_theta * cos_theta);
            auto phi = 2.f * pi * u[1];
            return {sin_theta * std::cosf(phi), cos_theta, sin_theta * std::sin(phi)};
        }

        auto pdf() const noexcept -> f32 {
            return 1.f / (4.f * pi);
        }
    };

    struct Hemisphere_Distribution final {
        Hemisphere_Distribution() noexcept = default;

        auto sample(Vector<f32, 2> const& u) const noexcept -> Vector<f32, 3> {
            auto z = u[0];
            auto r = math::sqrt(1 - z * z);
            auto phi = 2.f * pi * u[1];
            return {r * std::cosf(phi), r * std::sin(phi), z};
        }

        auto pdf() const noexcept -> f32 {
            return 1.f / (2.f * pi);
        }
    };

    struct Cosine_Hemisphere_Distribution final {
        Cosine_Hemisphere_Distribution() noexcept = default;

        auto sample(math::Vector<f32, 2> const& u) const noexcept -> math::Vector<f32, 3> {
            auto distr = Unifrom_Disk_Distribution{};
            auto d = distr.sample(u);
            return {d[0], math::sqrt(1.f - math::sqr(d[0]) - math::sqr(d[1])), d[1]};
        }

        auto pdf(f32 cos_theta) const noexcept -> f32 {
            return cos_theta / math::pi;
        }
    };
}
