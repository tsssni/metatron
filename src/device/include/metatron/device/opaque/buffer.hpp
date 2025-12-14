#pragma once
#include <metatron/device/command/queue.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::command {
    struct Buffer;
}

namespace mtt::opaque {
    struct Buffer final: stl::capsule<Buffer> {
        enum struct State {
            local,
            visible,
            twin,
        };

        struct View final {
            mut<Buffer> ptr;
            uptr offset;
            usize size;
        };

        State state;
        command::Queue::Type type;

        mut<byte> ptr = nullptr;
        uptr addr = 0;
        usize size = 0;
        std::vector<uv2> dirty = {};

        struct Descriptor final {
            mut<command::Buffer> cmd;
            mut<byte> ptr = nullptr;
            State state = State::local;
            usize size = 0;
            u64 flags = 0;
        };

        struct Impl;
        Buffer() noexcept = default;
        Buffer(cref<Descriptor> desc) noexcept;
        Buffer(rref<Buffer> rhs) noexcept;
        auto operator=(rref<Buffer> rhs) noexcept -> ref<Buffer>;
        ~Buffer() noexcept;
        operator View() noexcept;
    };
}
