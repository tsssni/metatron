#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/sampler/sampler.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/spectra/stochastic.hpp>

namespace mtt::monte_carlo {
    struct Context final {
        view<accel::Acceleration> accel;
        view<emitter::Emitter> emitter;
        mut<sampler::Sampler> sampler;
        fv4 lambda;
        math::Ray_Differential ray_differential;
        math::Ray_Differential default_differential;
        math::Transform render_to_camera;
        uzv2 pixel;
        usize sample_index;
        usize max_depth;
    };

    MTT_POLY_METHOD(integrator_sample, sample);

    struct Integrator final: pro::facade_builder
    ::add_convention<integrator_sample, auto (
        Context ctx
    ) const noexcept -> opt<spectra::Stochastic_Spectrum>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
