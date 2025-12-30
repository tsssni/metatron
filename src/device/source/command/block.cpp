#include <metatron/device/command/block.hpp>
#include <metatron/device/command/buffer.hpp>

namespace mtt::command {
    auto Blocks::allocate(usize size) noexcept -> opaque::Buffer::View {
        if (next + size > blocks.size() * block_size) {
            auto desc = opaque::Buffer::Descriptor{
                .state = opaque::Buffer::State::visible,
                .type = cmd->type,
                .size = block_size,
            };
            blocks.push_back(make_obj<opaque::Buffer>(desc));
            next = (blocks.size() - 1) * block_size;
        }

        auto offset = next % block_size;
        next += size;
        return {blocks.back().get(), offset, size};
    }

    auto Blocks::clear() noexcept -> void {
        next = 0;
    }
}
