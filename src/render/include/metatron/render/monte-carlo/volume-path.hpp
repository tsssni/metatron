#pragma once
#include <metatron/render/monte-carlo/integrator.hpp>

namespace mtt::monte_carlo {
    struct Volume_Path_Integrator final {
        // null scattering: https://cs.dartmouth.edu/~wjarosz/publications/miller19null.html
        // mis method: https://pbr-book.org/4ed/Light_Transport_II_Volume_Rendering/Volume_Scattering_Integrators
        auto sample(
            Context context,
            view<accel::Acceleration> accel,
            view<emitter::Emitter> emitter,
            mut<sampler::Sampler> sampler
        ) const noexcept -> std::optional<spectra::Stochastic_Spectrum>;
    };
}
