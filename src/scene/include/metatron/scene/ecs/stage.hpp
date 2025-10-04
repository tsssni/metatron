#pragma once
#include <metatron/scene/ecs/daemon.hpp>
#include <vector>

namespace mtt::ecs {
    struct Hierarchy;

    struct Stage final {
        std::vector<mut<Daemon>> daemons;
        auto init() noexcept -> void;
        auto update() noexcept -> void;
    };
}
