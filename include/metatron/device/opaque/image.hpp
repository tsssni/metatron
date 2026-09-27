#pragma once
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/resource/muldim/image.hpp>

namespace mtt::opaque {
    struct Image final: stl::capsule<Image> {
        enum struct State {
            samplable,
            storable,
        };

        struct View final {
            Image* ptr;
            uv2 mip;
            uv2 offset;
            uv2 size;
        };

        State state;
        u32 width;
        u32 height;
        u32 mips;
        std::vector<std::unique_ptr<Buffer>> host;

        struct Descriptor final {
            muldim::Image* image;
            State state = State::samplable;
            command::Type type = command::Type::render;
        };

        struct Impl;
        Image(Descriptor const& desc) noexcept;
        operator View() noexcept;
    };
}
