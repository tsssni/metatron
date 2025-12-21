#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::math {
    struct Bounding_Box final {
        fv3 p_min = {high<f32>};
        fv3 p_max = {low<f32>};
    };

    auto constexpr inside(cref<fv3> p, cref<Bounding_Box> bbox) noexcept -> bool {
        return math::all([](f32 x, f32 y, f32 z, auto) {
            return x >= y && x < z;
        }, p, bbox.p_min, bbox.p_max);
    }

    auto constexpr hit(
        cref<Ray> r,
        cref<Bounding_Box> bbox
    ) noexcept -> opt<fv2> {
        auto hit_min = (bbox.p_min - r.o) / r.d;
        auto hit_max = (bbox.p_max - r.o) / r.d;
        for (auto i = 0uz; i < 3uz; ++i)
            if (math::abs(r.d[i]) < epsilon<f32>) {
                hit_min[i] = -inf<f32>;
                hit_max[i] = +inf<f32>;
            } else if (hit_min[i] > hit_max[i]) {
                std::swap(hit_min[i], hit_max[i]);
            }

        auto t_enter = max(hit_min);
        auto t_exit = min(hit_max);
        if (t_exit < -epsilon<f32> || t_enter > t_exit + epsilon<f32>) return {};
        return fv2{t_enter, t_exit};
    }

    auto constexpr hitvi(
        cref<Ray> r,
        cref<Bounding_Box> bbox
    ) noexcept -> opt<std::tuple<f32, f32, usize, usize>> {
        auto hit_min = (bbox.p_min - r.o) / r.d;
        auto hit_max = (bbox.p_max - r.o) / r.d;
        for (auto i = 0uz; i < 3uz; ++i)
            if (math::abs(r.d[i]) < epsilon<f32>) {
                hit_min[i] = -inf<f32>;
                hit_max[i] = +inf<f32>;
            } else if (hit_min[i] > hit_max[i]) {
                std::swap(hit_min[i], hit_max[i]);
            }

        auto [t_enter, i_enter] = maxvi(hit_min);
        auto [t_exit, i_exit] = minvi(hit_max);
        if (t_exit < -epsilon<f32> || t_enter > t_exit + epsilon<f32>) return {};
        return std::make_tuple(t_enter, t_exit, i_enter, i_exit);
    }

    auto constexpr merge(
        cref<Bounding_Box> a,
        cref<Bounding_Box> b
    ) noexcept -> Bounding_Box {
        return Bounding_Box{
            .p_min = math::min(a.p_min, b.p_min),
            .p_max = math::max(a.p_max, b.p_max)
        };
    }

    auto constexpr area(cref<Bounding_Box> bbox) noexcept -> f32 {
        if(math::any([](f32 x, f32 y, usize) {
            return x >= y;
        }, bbox.p_min, bbox.p_max)) return 0.f;
        auto extent = bbox.p_max - bbox.p_min;
        return 2.f * (extent[0] * extent[1] + extent[1] * extent[2] + extent[2] * extent[0]);
    }

    auto constexpr operator|(cref<Transform> t, cref<Bounding_Box> bbox) -> Bounding_Box {
        auto tbox = Bounding_Box{};
        for (auto i = 0; i < 8; ++i) {
            auto p = fv3{};
            for (auto j = 0; j < 3; ++j) {
                auto k = (i >> j) & 0x1;
                p[j] = k == 0 ? bbox.p_min[j] : bbox.p_max[j];
            }
            p = t | expand(p, 1.f);
            tbox.p_min = min(tbox.p_min, p);
            tbox.p_max = max(tbox.p_max, p);
        }
        return tbox;
    }

    auto constexpr operator^(cref<Transform> t, cref<Bounding_Box> bbox) -> Bounding_Box {
        auto tbox = Bounding_Box{};
        for (auto i = 0; i < 8; ++i) {
            auto p = fv3{};
            for (auto j = 0; j < 3; ++j) {
                auto k = (i >> j) & 0x1;
                p[j] = k == 0 ? bbox.p_min[j] : bbox.p_max[j];
            }
            p = t ^ p;
            tbox.p_min = min(tbox.p_min, p);
            tbox.p_max = max(tbox.p_max, p);
        }
        return tbox;
    }
}
