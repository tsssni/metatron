#pragma once
#include <metatron/device/shader/layout.hpp>
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/device/opaque/image.hpp>
#include <metatron/device/opaque/grid.hpp>
#include <metatron/device/command/buffer.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::shader {
    template<typename T>
    struct Bindless final {
        usize offset;
        std::span<typename T::View> list;
    };

    struct Argument final: stl::capsule<Argument> {
        Set reflection;
        stl::table<u32> table;
        obj<opaque::Buffer> set;
        obj<opaque::Buffer> bindless;
        obj<opaque::Buffer> parameters;

        struct Descriptor final {
            std::string_view name;
        };

        struct Impl;
        Argument(cref<Descriptor> desc) noexcept;
        auto index(std::string_view field) noexcept -> u32;
    };
}
