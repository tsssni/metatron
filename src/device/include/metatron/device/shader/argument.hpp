#pragma once
#include <metatron/device/shader/layout.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::shader {
    struct Argument final: stl::capsule<Argument> {
        struct Impl;
        Argument(std::string_view name) noexcept;
    };
}
