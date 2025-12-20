#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::math {
    struct Polar_Disk_Distribution final {
        Polar_Disk_Distribution() noexcept = default;

        auto sample(fv2 u) const noexcept -> fv2 {
            auto r = math::sqrt(u[0]);
            auto theta = 2.f * pi * u[1];
            return r * fv2{std::cos(theta), std::sin(theta)};
        }

        auto pdf() const noexcept -> f32 {
            return 1.f / pi;
        }
    };

    struct Unifrom_Disk_Distribution final {
        Unifrom_Disk_Distribution() = default;

        auto sample(fv2 u) const noexcept -> fv2 {
            auto disk_u = u * 2.f - 1.f;
            auto r = 0.f;
            auto theta = 0.f;

            if (all([](f32 x, usize i){return x < epsilon<f32>;}, abs(disk_u))) {
                return {0.f};
            } else if (abs(disk_u[0]) > abs(disk_u[1])) {
                r = disk_u[0];
                theta = pi / 4.f * guarded_div(disk_u[1], disk_u[0]);
            } else {
                r = disk_u[1];
                theta = pi / 2.f - pi / 4.f * guarded_div(disk_u[0], disk_u[1]);
            }

            return r * fv2{std::cos(theta), std::sin(theta)};
        }

        auto pdf() const noexcept -> f32 {
            return 1.f / pi;
        }
    };
}
