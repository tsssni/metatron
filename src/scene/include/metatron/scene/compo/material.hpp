#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <unordered_map>

namespace mtt::compo {
    struct Interface_Bsdf final {
        i32 interface = 0;
    };

    struct Physical_Bsdf final {
        i32 physical = 0;
    };

    using Bsdf = std::variant<
        Interface_Bsdf,
        Physical_Bsdf
    >;

    struct Material final {
        Bsdf bsdf;
        std::unordered_map<std::string, ecs::Entity> spectrum_textures;
        std::unordered_map<std::string, ecs::Entity> vector_textures;
        std::unordered_map<std::string, ecs::Entity> samplers;
    };
}
