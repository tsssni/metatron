#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/sphere.hpp>

namespace mtt::math {
    struct Cone_Distribution final {
        Cone_Distribution() noexcept = default;
        Cone_Distribution(f32 cos_theta_max) noexcept: cos_theta_max(cos_theta_max) {}

        auto sample(cref<fv2> u) const noexcept -> fv3 {
            auto cos_theta = 1.f - u[0] * (1.f - cos_theta_max);
            auto phi = u[1] * 2.f * pi;
            return unit_spherical_to_cartesian(cos_theta, phi);
        }

        auto pdf() const noexcept -> f32 {
            return 1.f / (2.f * pi * (1.f - cos_theta_max));
        };

    private:
        f32 cos_theta_max;
    };
}
