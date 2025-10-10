#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/constant.hpp>

namespace mtt::math {
    struct Bounding_Box final {
        Vector<f32, 3> p_min{high<f32>};
        Vector<f32, 3> p_max{low<f32>};
    };

    auto inline constexpr inside(math::Vector<f32, 3> const& p, Bounding_Box const& bbox) noexcept -> bool {
        return math::all([](f32 x, f32 y, f32 z, auto) {
            return x >= y && x < z;
        }, p, bbox.p_min, bbox.p_max);
    }

    auto inline constexpr hit(
        Ray const& r,
        Bounding_Box const& bbox
    ) noexcept -> std::optional<math::Vector<f32, 2>> {
        auto hit_min = (bbox.p_min - r.o) / r.d;
        auto hit_max = (bbox.p_max - r.o) / r.d;
        for (auto i = 0uz; i < 3uz; ++i) {
            if (math::abs(r.d[i]) < epsilon<f32>) {
                hit_min[i] = -inf<f32>;
                hit_max[i] = +inf<f32>;
            } else if (hit_min[i] > hit_max[i]) {
                std::swap(hit_min[i], hit_max[i]);
            }
        }

        auto t_enter = max(hit_min);
        auto t_exit = min(hit_max);
        if (t_exit < -epsilon<f32> || t_enter > t_exit + epsilon<f32>) {
            return {};
        } else {
            return math::Vector<f32, 2>{t_enter, t_exit};
        }
    }

    auto inline constexpr hitvi(
        Ray const& r,
        Bounding_Box const& bbox
    ) noexcept -> std::optional<std::tuple<f32, f32, usize, usize>> {
        auto hit_min = (bbox.p_min - r.o) / r.d;
        auto hit_max = (bbox.p_max - r.o) / r.d;
        for (auto i = 0uz; i < 3uz; ++i) {
            if (math::abs(r.d[i]) < epsilon<f32>) {
                hit_min[i] = -inf<f32>;
                hit_max[i] = +inf<f32>;
            } else if (hit_min[i] > hit_max[i]) {
                std::swap(hit_min[i], hit_max[i]);
            }
        }

        auto [t_enter, i_enter] = maxvi(hit_min);
        auto [t_exit, i_exit] = minvi(hit_max);
        if (t_exit < -epsilon<f32> || t_enter > t_exit + epsilon<f32>) {
            return {};
        } else {
            return std::make_tuple(t_enter, t_exit, i_enter, i_exit);
        }
    }

    auto inline constexpr merge(
        Bounding_Box const& a,
        Bounding_Box const& b
    ) noexcept -> Bounding_Box {
        return Bounding_Box{
            .p_min = math::min(a.p_min, b.p_min),
            .p_max = math::max(a.p_max, b.p_max)
        };
    }

    auto inline constexpr area(Bounding_Box const& bbox) noexcept -> f32 {
        if(math::any([](f32 x, f32 y, usize) {
                return x >= y;
            }, bbox.p_min, bbox.p_max)
        ) {
            return 0.f;
        }
        auto extent = bbox.p_max - bbox.p_min;
        return 2.f * (extent[0] * extent[1] + extent[1] * extent[2] + extent[2] * extent[0]);
    }
}
