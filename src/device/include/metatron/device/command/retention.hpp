#pragma once
#include <metatron/device/command/queue.hpp>
#include <metatron/device/command/block.hpp>
#include <queue>

namespace mtt::command {
    struct Retention final: stl::singleton<Retention>, stl::capsule<Retention> {
        struct Impl;
        Retention() noexcept;
        ~Retention() noexcept;
    };
}
