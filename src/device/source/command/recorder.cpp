#include <metatron/device/command/recorder.hpp>
#include <metatron/core/math/bit.hpp>
#include <metatron/core/stl/ranges.hpp>

namespace mtt::command {
    auto Recorder::bind(mut<opaque::Buffer> buffer) noexcept -> void {
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
            auto size = buffer->dirty[1] - buffer->dirty[0];
            auto [bid, offset] = allocate(size);
            auto& block = blocks[bid];
            update_buffers.push_back(buffer->flush(block.get(), offset));
            update_dsts.push_back(buffer);
        }
    }

    auto Recorder::bind(mut<opaque::Image> image) noexcept -> void {
        auto& retention = Retentions::instance().retentions[u32(type)];
        auto idx = timestamp % Retention::num_recorder;
        auto& image_buffers = retention.image_buffers[idx];
        auto& blocks = retention.blocks[idx];

        if (image->type != type) image_transfers[u32(image->type)].push_back(image);
        auto& wait = wait_timestamps[u32(image->type)];
        wait = std::max(wait, image->timestamp);
        image->type = type;
        image->timestamp = timestamp;

        if (true
        && image->state == opaque::Image::State::sampled
        && !image->host.empty()) {
            image_buffers.push_back(std::move(image->host));
            image_dsts.push_back(image);
        }
    }

    auto Recorder::allocate(usize size) noexcept -> uzv2 {
        auto& retention = Retentions::instance().retentions[u32(type)];
        auto idx = timestamp % Retention::num_recorder;
        auto& blocks = retention.blocks[idx];

        if (next + size >= blocks.size() * Retention::block_size) {
            auto desc = opaque::Buffer::Descriptor{
                .type = type,
                .state = opaque::Buffer::State::visible,
                .size = Retention::block_size,
            };
            blocks.push_back(make_obj<opaque::Buffer>(desc));
        }

        auto bid = blocks.size() - 1;
        auto start = bid * Retention::block_size;
        auto offset = next + size > start ? start : next;
        next = start + offset + size;
        return {bid, offset};
    }
}
