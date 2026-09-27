#pragma once
#include <metatron/render/monte-carlo/radiative.hpp>

namespace mtt::monte_carlo {
    auto init() noexcept -> void;

    struct Integrator final: stl::polymorph<Integrator
    , Radiative_Integrator> {
        using polymorph::polymorph;
        auto trace(Context& ctx) noexcept -> void {
            return visit([&](auto* p) noexcept { return p->trace(ctx); });
        }
    };
}
