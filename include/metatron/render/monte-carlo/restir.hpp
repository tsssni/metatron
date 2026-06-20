#pragma once
#include <metatron/render/monte-carlo/context.hpp>
#include <metatron/core/math/reservoir.hpp>

namespace mtt::monte_carlo {
    struct Restir_Integrator final {
        struct Path final {
            math::Reservoir reservoir;
            spectra::Stochastic_Spectrum Li;
            accel::proxy::Divider divider;
            fv4 beta;
            fv3 p;
            fv3 wi;
            fv2 pdf;
            u32 pixel;
            u32 depth = 0; // 0 means unavailable for reconnnection

            auto operator()(spectra::Stochastic_Spectrum Li, f32 W, f32 u) noexcept -> void {
                auto y = math::max(Li.value);
                auto r = math::Reservoir{
                    .p_hat = y,
                    .M = 1.f,
                    .W = W,
                };
                r.shift(1.f);
                if (reservoir(r, u)) this->Li = Li;
            }
        };

        u32 reuse_iterations = 5;
        u32 reuse_confidence = 32;
        u32 spatial_samples = 4;

        // gris: https://graphics.cs.utah.edu/research/projects/gris/
        auto sample(ref<Context> ctx) const noexcept -> opt<spectra::Stochastic_Spectrum>;
    };
}
