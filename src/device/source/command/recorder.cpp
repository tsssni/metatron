#include <metatron/device/command/recorder.hpp>
#include <metatron/core/math/bit.hpp>

namespace mtt::command {
    auto Recorder::bind(mut<opaque::Buffer> buffer) noexcept -> void {
        auto& queue = Queues::instance().queues[u32(type)];
        auto& retention = Retentions::instance().retentions[u32(type)];
        auto idx = timestamp % Retention::num_recorder;
        auto& update_buffers = retention.update_buffers[idx];
        auto& stage_buffers = retention.stage_buffers[idx];
        auto& blocks = retention.blocks[idx];

        if (buffer->type != type) buffer_transfers[u32(buffer->type)].push_back(buffer);
        auto& wait = wait_timestamps[u32(buffer->type)];
        wait = std::max(wait, buffer->timestamp);
        buffer->type = type;
        buffer->timestamp = timestamp;

        if (true
        && buffer->state == opaque::Buffer::State::local
        && buffer->ptr) {
            stage_buffers.push_back(buffer->upload());
            stage_dsts.push_back(buffer);
        } else if (true
        && buffer->state == opaque::Buffer::State::writable
        && buffer->dirty[1] > buffer->dirty[0]) {
            auto constexpr block_size = 1 << 24;
            if (next >= blocks.size() * block_size) {
                auto desc = opaque::Buffer::Descriptor{
                    .type = type,
                    .state = opaque::Buffer::State::visible,
                    .size = block_size,
                };
                blocks.push_back(make_obj<opaque::Buffer>(desc));
            }
            auto start = (blocks.size() - 1) * block_size;
            auto offset = next - start;
            auto& block = blocks[next / block_size];
            next += buffer->dirty[1] = buffer->dirty[0];
            update_buffers.push_back(buffer->flush(block.get(), offset));
            update_dsts.push_back(buffer);
        }
    }
}
