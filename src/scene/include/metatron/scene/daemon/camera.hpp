#pragma once
#include <metatron/scene/ecs/entity.hpp>
#include <metatron/core/math/transform.hpp>

namespace mtt::daemon {
    struct Camera_Daemon final {
        ecs::Entity camera{ecs::null};
        auto init() noexcept -> void;
        auto update() noexcept -> void;
    };
}
