#pragma once
#include <metatron/render/accel/accel.hpp>
#include <metatron/render/emitter/emitter.hpp>
#include <metatron/render/monte-carlo/integrator.hpp>
#include <metatron/render/photo/camera.hpp>

namespace mtt::render {

struct Monte_Carlo_Context final {
    Integrator integrator;
    Acceleration accel;
    Emitter emitter;
    Sampler sampler;
    Filter filter;
    Lens lens;
    Film* film = nullptr;
    core::Transform const* camera = nullptr;

    u32 seed;
    u32 sample_index;
};

}
