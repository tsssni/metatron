#include <metatron/device/command/buffer.hpp>

namespace mtt::command {
    auto Blocks::allocate(usize size) noexcept -> opaque::Buffer::View {
        auto offset = next % block_size;
        if (offset + size > block_size) {
            next = (next / block_size + 1) * block_size;
            offset = 0;
        }
        auto idx = next / block_size;
        while (blocks.size() <= idx) {
            blocks.push_back(make_desc<opaque::Buffer>({
                .state = opaque::Buffer::State::visible,
                .type = cmd->type,
                .size = block_size,
            }));
        }
        next += size;
        return {blocks[idx].get(), offset, size};
    }

    auto Blocks::clear() noexcept -> void {
        next = 0;
    }
}
