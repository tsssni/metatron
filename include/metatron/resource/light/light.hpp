#pragma once
#include <metatron/resource/light/area.hpp>
#include <metatron/resource/light/environment.hpp>
#include <metatron/resource/light/atmosphere.hpp>

namespace mtt::light {
    auto init() noexcept -> void;

    struct Light final: stl::polynomial<Light
    , Area_Light
    , Environment_Light
    , Atmosphere_Light> {
        using polynomial::polynomial;

        auto operator()(math::Ray const& r, fv4 const& lambda) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return (*p)(r, lambda); });
        }
        auto sample(math::Context const& ctx, fv2 const& u) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto flags() const noexcept -> Flags {
            return visit([&](auto* p) noexcept { return p->flags(); });
        }
    };
}
