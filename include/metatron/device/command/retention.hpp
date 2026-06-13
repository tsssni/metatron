#pragma once
#include <metatron/device/command/block.hpp>

namespace mtt::command {
    struct Retention final: stl::singleton<Retention>, stl::capsule<Retention> {
        struct Impl;
        Retention() noexcept;
        ~Retention() noexcept;
    };
}
