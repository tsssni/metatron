#pragma once
#include <metatron/device/shader/layout.hpp>
#include <metatron/device/opaque/image.hpp>
#include <metatron/device/opaque/grid.hpp>
#include <metatron/device/command/buffer.hpp>

namespace mtt::shader {
    template<typename T>
    struct Bindless final {
        usize offset;
        std::span<typename T::View> list;
    };

    struct Argument final: stl::capsule<Argument> {
        Set reflection;
        obj<opaque::Buffer> set;

        struct Descriptor final {
            std::string_view name;
            usize count;
        };

        struct Impl;
        Argument(cref<Descriptor> desc) noexcept;
    };
}
