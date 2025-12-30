#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/render/sampler/proxy.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/spectra/stochastic.hpp>

namespace mtt::monte_carlo {
    struct Context final {
        view<accel::Acceleration> accel;
        view<emitter::Emitter> emitter;
        mut<sampler::Proxy_Sampler> sampler;
        fv4 lambda;
        math::Ray_Differential ray_differential;
        math::Ray_Differential default_differential;
        math::Transform render_to_camera;
        uv2 pixel;
        u32 sample_index;
        u32 max_depth;
    };

    MTT_POLY_METHOD(integrator_sample, sample);

    struct Integrator final: pro::facade_builder
    ::add_convention<integrator_sample, auto (
        ref<Context> ctx
    ) const noexcept -> opt<spectra::Stochastic_Spectrum>>
    ::add_skill<pro::skills::as_view>
    ::build {};
}
