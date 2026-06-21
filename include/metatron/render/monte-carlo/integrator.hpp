#pragma once
#include <metatron/render/monte-carlo/radiative.hpp>
#include <metatron/render/monte-carlo/restir.hpp>

namespace mtt::monte_carlo {
    struct Integrator final: stl::polynomial<Integrator
    , Radiative_Integrator
    , Restir_Integrator> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;
        auto acquire(cref<Context> ctx, cref<Resources> res) noexcept -> void {
            return visit([&](auto* p) noexcept { return p->acquire(ctx, res); });
        }
        auto release() noexcept -> void {
            return visit([&](auto* p) noexcept { return p->release(); });
        }
        auto trace(ref<Context> ctx) const noexcept -> void {
            return visit([&](auto* p) noexcept { return p->trace(ctx); });
        }
        auto wave(ref<Context> ctx) const noexcept -> void {
            ctx.integrator = u32(*this);
            return visit([&](auto* p) noexcept { return p->wave(ctx); });
        }
    };
}
