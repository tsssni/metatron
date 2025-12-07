#pragma once
#include <metatron/device/command/queue.hpp>
#include <metatron/core/stl/capsule.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::opaque {
    struct Buffer final: stl::capsule<Buffer> {
        enum struct State {
            local,
            visible,
            twin,
        };

        command::Queue::Type type;
        State state;
        u64 timestamp = 0;

        mut<byte> ptr;
        uptr addr;
        usize size;
        uv2 dirty = {math::maxv<u32>, math::minv<u32>};

        struct Descriptor final {
            command::Queue::Type type;
            State state = State::local;
            mut<byte> ptr = nullptr;
            usize size = 0;
            u64 flags = 0;
        };

        struct Impl;
        Buffer() noexcept = default;
        Buffer(cref<Descriptor> desc) noexcept;
        ~Buffer() noexcept;
        auto upload() noexcept -> obj<Buffer>;
        auto flush(view<Buffer> dst, usize offset) noexcept -> obj<Buffer>;
    };
}
