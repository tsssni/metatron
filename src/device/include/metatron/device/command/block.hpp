#pragma once
#include <metatron/device/opaque/buffer.hpp>

namespace mtt::command {
    auto constexpr block_size = 1 << 26;

    struct Buffer;

    struct Blocks final {
        mut<Buffer> cmd;
        auto allocate(usize size) noexcept -> opaque::Buffer::View;
        auto clear() noexcept -> void;

    private:
        uptr next = 0;
        std::vector<obj<opaque::Buffer>> blocks;
    };
}
