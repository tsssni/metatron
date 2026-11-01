#pragma once
#include <metatron/render/light/light.hpp>

namespace mtt::render {

struct Emitter_Interaction final {
    render::Light light;
    core::Transform const* local_to_render;
    f32 pdf;
};

}
