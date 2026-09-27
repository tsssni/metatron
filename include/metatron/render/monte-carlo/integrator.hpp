#pragma once
#include <metatron/render/monte-carlo/radiative.hpp>

namespace mtt::monte_carlo {
    auto init() noexcept -> void;

    struct Integrator final: stl::polynomial<Integrator
    , Radiative_Integrator> {
        using polynomial::polynomial;
        auto trace(ref<Context> ctx) noexcept -> void {
            return visit([&](auto* p) noexcept { return p->trace(ctx); });
        }
    };
}
