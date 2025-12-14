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
            uzv2 mip;
            uzv2 offset;
            uzv2 size;
        };

        command::Queue::Type type;
        State state;

        u32 width;
        u32 height;
        u32 mips;
        std::vector<obj<Buffer>> host;

        struct Descriptor final {
            mut<command::Buffer> cmd;
            mut<muldim::Image> image;
            State state = State::samplable;
        };

        struct Impl;
        Image(cref<Descriptor> desc) noexcept;
        operator View() noexcept;
    };
}
