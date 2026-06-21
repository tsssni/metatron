#pragma once
#include <metatron/render/monte-carlo/context.hpp>
#include <metatron/device/shader/argument.hpp>
#include <metatron/device/shader/pipeline.hpp>
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

        struct Descriptor final {
            u32 reuse_iterations = 5;
            u32 reuse_confidence = 32;
            u32 spatial_samples = 4;
        };
        Restir_Integrator(cref<Descriptor> desc) noexcept;
        Restir_Integrator() noexcept = default;

        // gris: https://graphics.cs.utah.edu/research/projects/gris/
        auto acquire(cref<Context> ctx, cref<Resources> res) noexcept -> void;
        auto release() noexcept -> void;
        auto trace(ref<Context> ctx) const noexcept -> void;
        auto wave(ref<Context> ctx) const noexcept -> void;
        auto sample(ref<Ray> r) const noexcept -> opt<spectra::Stochastic_Spectrum>;

    private:
        obj<shader::Pipeline> integrate;
        obj<shader::Argument> constants;
        std::array<buf<Path>, 2> pathes;
        u32 reuse_iterations;
        u32 reuse_confidence;
        u32 spatial_samples;
    };
}
