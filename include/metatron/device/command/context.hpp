#pragma once
#include <metatron/core/stl/singleton.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::command {
    struct Context final: stl::singleton<Context>, stl::capsule<Context> {
        struct Impl;
        auto static init() noexcept -> void;
    };
}
