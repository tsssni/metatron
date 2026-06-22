#pragma once
#include <metatron/render/monte-carlo/context.hpp>
#include <metatron/device/shader/argument.hpp>
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/core/math/reservoir.hpp>

namespace mtt::monte_carlo {
    struct Restir_Integrator final {
        struct Path final {
            math::Reservoir r;
            spectra::Stochastic_Spectrum Li;
            fv4 beta;
            fv3 p;
            f32 t;
            fv3 wi;
            f32 cos_theta;
            fv2 pdf;
            f32 J;
            u32 pixel;
            u32 depth = 0;
            uv3 padding;

            auto estimate(spectra::Stochastic_Spectrum Li, f32 W, f32 u) noexcept -> void;
            auto merge(cref<Path> p, f32 u) noexcept -> void;
        };

        struct Descriptor final {
            u32 reuse_iterations = 3;
            u32 spatial_samples = 3;
            u32 spatial_radius = 20;
        };
        Restir_Integrator(cref<Descriptor> desc) noexcept;
        Restir_Integrator() noexcept = default;

        // gris: https://graphics.cs.utah.edu/research/projects/gris/
        auto upload(cref<Context> ctx) noexcept -> void;
        auto acquire(cref<Context> ctx, cref<Resources> res) noexcept -> void;
        auto release() noexcept -> void;
        auto trace(ref<Context> ctx) noexcept -> void;
        auto wave(ref<Context> ctx) const noexcept -> void;
        auto sample(ref<Ray> r) const noexcept -> opt<Path>;
        auto replay(ref<Ray> r, cref<Path> np, f32 u) const noexcept -> opt<Path>;

    private:
        obj<shader::Pipeline> tracer;
        obj<shader::Pipeline> reuser;
        obj<shader::Argument> constants;
        std::array<buf<Path>, 2> pathes;
        u32 reuse_iterations;
        u32 spatial_samples;
        u32 spatial_radius;
    };
}
