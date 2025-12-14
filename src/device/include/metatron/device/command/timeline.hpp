#pragma once
#include <metatron/core/stl/capsule.hpp>

namespace mtt::command {
    struct Timeline final: stl::capsule<Timeline> {
        struct Impl;
        Timeline() noexcept;
        auto wait(u64 count, u64 timeout = 1e9) noexcept -> bool;
        auto signal(u64 count) noexcept -> void;
    };
}
