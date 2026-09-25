#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/photo/camera.hpp>

namespace mtt::monte_carlo {
    struct Context final {
        accel::Acceleration accel;
        emitter::Emitter emitter;
        sampler::Sampler sampler;
        filter::Filter filter;
        photo::Lens lens;
        photo::proxy::Film film;

        u32 seed;
        u32 sample_index;

        Context() noexcept;
    };

    struct Ray final {
        accel::Acceleration accel;
        emitter::Emitter emitter;
        sampler::proxy::Sampler sampler;
        fv4 lambda;
        math::Ray_Differential ray_differential;
        u32 max_bounce;
    };
}
