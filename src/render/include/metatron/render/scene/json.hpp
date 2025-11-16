#pragma once
#include <metatron/render/scene/entity.hpp>
#include <glaze/glaze.hpp>

namespace mtt::scene {
    struct json final {
        scene::Entity entity;
        std::string type;
        glz::raw_json serialized;
    };
}


