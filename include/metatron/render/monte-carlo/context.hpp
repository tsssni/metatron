#pragma once
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/photo/camera.hpp>
#include <metatron/device/shader/argument.hpp>

namespace mtt::monte_carlo {
    struct Resources final {
        obj<shader::Argument> resources;
        obj<shader::Argument> textures;
        obj<shader::Argument> grids;
    };

    struct Context final {
        accel::Acceleration accel;
        emitter::Emitter emitter;
        sampler::Sampler sampler;
        filter::Filter filter;
        photo::Lens lens;
        photo::proxy::Film film;

        mut<command::Buffer> render = nullptr;
        mut<opaque::Image> image = nullptr;

        u32 seed;
        u32 sample_index;
        u32 integrator = 0;

        Context() noexcept;
    };

    struct Ray final {
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
