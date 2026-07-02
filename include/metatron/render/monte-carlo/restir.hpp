#pragma once
#include <metatron/render/monte-carlo/context.hpp>
#include <metatron/device/shader/argument.hpp>
#include <metatron/device/shader/pipeline.hpp>
#include <metatron/core/math/reservoir.hpp>

namespace mtt::monte_carlo {
    struct Restir_Integrator final {
        struct Estimation final {
            spectra::Stochastic_Spectrum Li;
            fv4 beta;
            f32 mis;
            f32 p_e;
            fv3 wi;
            u32 end;
            f32 W;
            f32 u;
        };

        struct Path final {
            math::Reservoir r;
            fv4 Li;
            fv4 beta;
            fv4 gamma;
            fv3 p;
            u32 pixel;
            fv3 wi;
            f32 J;
            fv3 we;
            f32 mis;
            fv3 pdf;
            u32 depth;

            auto estimate(Estimation&& e) noexcept -> void;
            auto merge(cref<Path> p, f32 u) noexcept -> void;
        };

        struct Constants {
            accel::Acceleration accel;
            emitter::Emitter emitter;
            sampler::Sampler sampler;
            filter::Filter filter;
            photo::Lens lens;
            photo::proxy::Film film;
            math::Transform ct;
            u32 seed;
            u32 sample_index;
            u32 iter;
            u32 integrator;
            opaque::Image::View image;
        };

        struct Descriptor final {
            u32 reuse_iterations = 3;
            u32 spatial_samples = 3;
            u32 spatial_radius = 20;
            f32 near_field_distance = 1e-1f;
        };
        Restir_Integrator(cref<Descriptor> desc) noexcept;
        Restir_Integrator() noexcept = default;

        // gris: https://graphics.cs.utah.edu/research/projects/gris/
        auto upload(ref<Context> ctx) noexcept -> void;
        auto acquire(ref<Context> ctx, cref<Resources> res) noexcept -> void;
        auto release() noexcept -> void;
        auto trace(ref<Context> ctx) noexcept -> void;
        auto wave(ref<Context> ctx) const noexcept -> void;
        auto sample(ref<Ray> r) const noexcept -> opt<Path>;
        auto replay(ref<Ray> r, cref<Path> np, f32 u) const noexcept -> opt<std::tuple<Path, f32>>;

    private:
        std::array<buf<Path>, 2> pathes;
        u32 reuse_iterations;
        u32 spatial_samples;
        u32 spatial_radius;
        f32 near_field_distance;

        obj<shader::Pipeline> integrate;
        obj<shader::Pipeline> restir;
        obj<shader::Argument> arguments;
        obj<Constants> constants;
    };
}
