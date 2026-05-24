#pragma once
#include <metatron/render/monte-carlo/context.hpp>
#include <metatron/render/monte-carlo/radiative.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::monte_carlo {
    struct Integrator final: stl::polynomial<Integrator
    , Radiative_Integrator> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto sample(ref<Context> ctx) const noexcept -> opt<spectra::Stochastic_Spectrum> {
            return visit([&](auto* p) noexcept { return p->sample(ctx); });
        }
    };
}
