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

        command::Queue::Type type;
        State state;
        u64 timestamp = 0;

        u32 width;
        u32 height;
        u32 depth;
        obj<Buffer> host;

        struct Descriptor final {
            command::Queue::Type type;
            State state;
            mut<muldim::Grid> grid;
        };

        struct Impl;
        Grid(cref<Descriptor> desc) noexcept;
    };
}
