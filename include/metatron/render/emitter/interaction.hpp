#pragma once
#include <metatron/resource/light/light.hpp>

namespace mtt::emitter {
    struct Interaction final {
        light::Light light;
        math::proxy::Transform local_to_render;
        f32 pdf;
    };
}
