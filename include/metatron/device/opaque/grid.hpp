#pragma once
#include <metatron/device/opaque/buffer.hpp>
#include <metatron/resource/muldim/grid.hpp>

namespace mtt::opaque {
    struct Grid final: stl::capsule<Grid> {
        enum struct State {
            readonly,
            writable,
        };

        struct View final {
            Grid* ptr;
            uv3 offset;
            uv3 size;
        };

        State state;
        u32 width;
        u32 height;
        u32 depth;
        std::unique_ptr<Buffer> host;

        struct Descriptor final {
            muldim::Grid* grid;
            State state = State::readonly;
            command::Type type = command::Type::render;
        };

        struct Impl;
        Grid(Descriptor const& desc) noexcept;
        operator View() noexcept;
    };
}
