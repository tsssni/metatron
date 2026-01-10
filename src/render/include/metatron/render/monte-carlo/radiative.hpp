#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>

namespace mtt::monte_carlo {
    struct Radiative_Integrator final {
        struct Descriptor final {};
        Radiative_Integrator(cref<Descriptor>) noexcept;
        Radiative_Integrator() noexcept = default;

        // null scattering: https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html
        // mis method: https://pbr-book.org/4ed/Light_Transport_II_Volume_Rendering/Volume_Scattering_Integrators
        auto sample(ref<Context> ctx) const noexcept -> opt<spectra::Stochastic_Spectrum>;

    private:
        u32 padding = 0u;
    };
}
