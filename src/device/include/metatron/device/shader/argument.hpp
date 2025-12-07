#pragma once
#include <metatron/device/shader/layout.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::shader {
    struct Argument final: stl::capsule<Argument> {
        Set reflection;
        obj<opaque::Buffer> uniform;
        obj<opaque::Buffer> set;

        struct Impl;
        Argument(std::string_view name) noexcept;
    };
}
