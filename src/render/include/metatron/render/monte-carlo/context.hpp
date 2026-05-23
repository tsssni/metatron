#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/spectra/stochastic.hpp>

namespace mtt::monte_carlo {
    struct Context final {
        accel::Acceleration accel;
        emitter::Emitter emitter;
        sampler::proxy::Sampler sampler;
        fv4 lambda;
        math::Ray_Differential ray_differential;
        math::Ray_Differential default_differential;
        math::Transform render_to_camera;
        uv2 pixel;
        u32 sample_index;
        u32 max_depth;
    };
}
