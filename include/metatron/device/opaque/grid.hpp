#pragma once
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/resource/muldim/grid.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::opaque {
    struct Grid final: stl::capsule<Grid> {
        enum struct State {
            readonly,
            writable,
        };

        struct View final {
            mut<Grid> ptr;
            uv3 offset;
            uv3 size;
        };

        State state;
        u32 width;
        u32 height;
        u32 depth;
        obj<Buffer> host;

        struct Descriptor final {
            mut<muldim::Grid> grid;
            State state = State::readonly;
            command::Type type = command::Type::render;
        };

        struct Impl;
        Grid(cref<Descriptor> desc) noexcept;
        operator View() noexcept;
    };
}
