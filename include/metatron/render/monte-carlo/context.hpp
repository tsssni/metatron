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
        math::proxy::Transform camera;

        u32 seed;
        u32 sample_index;

        Context() noexcept;
    };
}
