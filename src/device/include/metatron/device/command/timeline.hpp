#pragma once
#include <metatron/core/math/constant.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::command {
    struct Timeline final: stl::capsule<Timeline> {
        struct Impl;
        Timeline() noexcept;
        auto wait(u64 count, u64 timeout = math::maxv<u64>) noexcept -> bool;
        auto signal(u64 count) noexcept -> void;
    };
}
