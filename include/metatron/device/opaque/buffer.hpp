#pragma once
#include <metatron/device/command/queue.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::opaque {
    struct Buffer final: stl::capsule<Buffer> {
        enum struct State {
            local,
            visible,
            twin,
        };

        struct View final {
            Buffer* ptr;
            uptr offset;
            usize size;
        };

        State state;
        byte* ptr = nullptr;
        uptr addr = 0;
        u32 size = 0;
        std::vector<uv2> dirty = {};

        struct Descriptor final {
            byte const* ptr = nullptr;
            State state = State::local;
            command::Type type = command::Type::render;
            usize alignment = 0;
            usize size = 0;
            u64 flags = 0;
        };

        struct Impl;
        Buffer() noexcept = default;
        Buffer(Descriptor const& desc) noexcept;
        Buffer(Buffer&& rhs) noexcept;
        auto operator=(Buffer&& rhs) noexcept -> Buffer&;
        operator View() noexcept;
    };
}
