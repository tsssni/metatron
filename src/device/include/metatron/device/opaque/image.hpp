#pragma once
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/resource/muldim/image.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::opaque {
    struct Image final: stl::capsule<Image> {
        enum struct State {
            samplable,
            storable,
        };

        struct View final {
            mut<Image> ptr;
            uv2 mip;
            uv2 offset;
            uv2 size;
        };

        State state;
        u32 width;
        u32 height;
        u32 mips;
        std::vector<obj<Buffer>> host;

        struct Descriptor final {
            mut<muldim::Image> image;
            State state = State::samplable;
            command::Type type = command::Type::render;
        };

        struct Impl;
        Image(cref<Descriptor> desc) noexcept;
        operator View() noexcept;
    };
}
