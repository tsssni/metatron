#pragma once
#include <metatron/device/opaque/buffer.hpp>

namespace mtt::command {
    auto constexpr block_size = 1 << 12;

    struct Buffer;
    struct Blocks final {
        Buffer* cmd;
        auto allocate(usize size) noexcept -> opaque::Buffer::View;
        auto clear() noexcept -> void;

    private:
        uptr next = 0;
        std::vector<std::unique_ptr<opaque::Buffer>> blocks;
    };
}
