#pragma once
#include <metatron/render/monte-carlo/context.hpp>

namespace mtt::monte_carlo {
    struct Restir_Integrator final {
        struct Descriptor final {};
        Restir_Integrator(cref<Descriptor>) noexcept;
        Restir_Integrator() noexcept = default;

        // gris: https://graphics.cs.utah.edu/research/projects/gris/
        auto sample(ref<Context> ctx) const noexcept -> opt<spectra::Stochastic_Spectrum>;

    private:
        u32 padding = 0u;
    };
}
