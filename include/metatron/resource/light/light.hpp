#pragma once
#include <metatron/resource/light/parallel.hpp>
#include <metatron/resource/light/point.hpp>
#include <metatron/resource/light/spot.hpp>
#include <metatron/resource/light/area.hpp>
#include <metatron/resource/light/environment.hpp>
#include <metatron/resource/light/atmosphere.hpp>

namespace mtt::light {
    struct Light final: stl::polynomial<Light
    , Parallel_Light
    , Point_Light
    , Spot_Light
    , Area_Light
    , Environment_Light
    , Atmosphere_Light> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto operator()(cref<math::Ray> r, cref<fv4> lambda) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return (*p)(r, lambda); });
        }
        auto sample(cref<math::Context> ctx, cref<fv2> u) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto flags() const noexcept -> Flags {
            return visit([&](auto* p) noexcept { return p->flags(); });
        }
    };
}
