#include "transfer.hpp"
#include "../command/buffer.hpp"
#include "../opaque/buffer.hpp"
#include "../opaque/image.hpp"
#include "../opaque/grid.hpp"

namespace mtt::encoder {
    using Buffer = opaque::Buffer;
    using Image = opaque::Image;
    using Grid = opaque::Grid;

    Transfer_Encoder::Transfer_Encoder(mut<command::Buffer> cmd) noexcept: cmd(cmd) {
        impl->encoder = cmd->impl->cmd->blitCommandEncoder();
    }
    auto Transfer_Encoder::submit() noexcept -> void { impl->encoder->endEncoding(); }

    auto Transfer_Encoder::upload(opaque::Buffer::View view) noexcept -> void {
        using State = Buffer::State;
        auto buffer = view.ptr;
        if (buffer->state == State::local && buffer->ptr) {
            auto uploaded = make_obj<Buffer>();
            auto buffer = view.ptr;
            uploaded->impl->device_buffer = std::move(buffer->impl->host_buffer);
            uploaded->state = Buffer::State::visible;
            uploaded->ptr = buffer->ptr;
            uploaded->size = buffer->size;
            buffer->ptr = nullptr;

            auto staged = Buffer::View(*uploaded);
            cmd->stages.push_back(std::move(uploaded));
            copy(view, staged);
        } else if (buffer->state == State::twin && !buffer->dirty.empty()) {
            auto buffer = view.ptr;
            auto dirty = std::move(buffer->dirty);
            auto sum = std::ranges::fold_left(dirty, 0, [](i32 x, uv2 y) {
                return x + y[1];
            });
            auto block = cmd->blocks.allocate(sum);
            auto uploaded = std::ranges::fold_left(dirty, 0, [&](i32 x, uv2 y) {
                std::memcpy(block.ptr->ptr + block.offset + x, buffer->ptr + y[0], y[1]);
                copy(
                    Buffer::View{buffer, y[0], y[1]},
                    Buffer::View{block.ptr, block.offset + x, y[1]}
                );
                return x + y[1];
            });
        }
    }

    auto Transfer_Encoder::upload(opaque::Image::View image) noexcept -> void {}
    auto Transfer_Encoder::upload(opaque::Grid::View grid) noexcept -> void {}

    auto Transfer_Encoder::copy(opaque::Buffer::View dst, opaque::Buffer::View src) noexcept -> void {
        impl->encoder->copyFromBuffer(
            src.ptr->impl->device_buffer.get(), src.offset,
            dst.ptr->impl->device_buffer.get(), dst.offset,
            dst.size
        );
    }

    auto Transfer_Encoder::copy(opaque::Image::View dst, opaque::Buffer::View src) noexcept -> void {
        auto width = math::max(dst.ptr->width >> dst.mip[0], 1u);
        auto height = math::max(dst.ptr->height >> dst.mip[0], 1u);
        impl->encoder->copyFromBuffer(
            src.ptr->impl->device_buffer.get(),
            src.offset, src.size / height, 0,
            {dst.size[0], dst.size[1], 1},
            dst.ptr->impl->texture.get(), 0, dst.mip[0],
            {dst.offset[0], dst.offset[1], 1}
        );
    }

    auto Transfer_Encoder::copy(opaque::Grid::View dst, opaque::Buffer::View src) noexcept -> void {
        impl->encoder->copyFromBuffer(
            src.ptr->impl->device_buffer.get(), src.offset,
            dst.size[0] * sizeof(f32),
            dst.size[0] * dst.size[1] * sizeof(f32),
            {dst.size[0], dst.size[1], dst.size[2]},
            dst.ptr->impl->texture.get(), 0, 0,
            {dst.offset[0], dst.offset[1], dst.offset[2]}
        );
    }

    auto Transfer_Encoder::copy(opaque::Buffer::View dst, opaque::Image::View src) noexcept -> void {
        impl->encoder->copyFromTexture(
            src.ptr->impl->texture.get(), 0, src.mip[0],
            {src.offset[0], src.offset[1], 0},
            {src.size[0], src.size[1], 1},
            dst.ptr->impl->device_buffer.get(),
            dst.offset, dst.size / src.size[1], 0
        );
    }

    auto Transfer_Encoder::copy(opaque::Buffer::View dst, opaque::Grid::View src) noexcept -> void {
        impl->encoder->copyFromTexture(
            src.ptr->impl->texture.get(), 0, 0,
            {src.offset[0], src.offset[1], src.offset[2]},
            {src.size[0], src.size[1], src.size[2]},
            dst.ptr->impl->device_buffer.get(), dst.offset,
            src.size[0] * sizeof(f32),
            src.size[0] * src.size[1] * sizeof(f32)
        );
    }

    auto Transfer_Encoder::copy(opaque::Image::View dst, opaque::Image::View src) noexcept -> void {
        auto width = math::max(src.ptr->width >> src.mip[0], 1u);
        auto height = math::max(src.ptr->height >> src.mip[0], 1u);
        impl->encoder->copyFromTexture(
            src.ptr->impl->texture.get(), 0, src.mip[0],
            {src.offset[0], src.offset[1], 0},
            {src.size[0], src.size[1], 1},
            dst.ptr->impl->texture.get(), 0, dst.mip[0],
            {dst.offset[0], dst.offset[1], 0}
        );
    }

    auto Transfer_Encoder::copy(opaque::Grid::View dst, opaque::Grid::View src) noexcept -> void {
        impl->encoder->copyFromTexture(
            src.ptr->impl->texture.get(), 0, 0,
            {src.offset[0], src.offset[1], src.offset[2]},
            {src.size[0], src.size[1], src.size[2]},
            dst.ptr->impl->texture.get(), 0, 0,
            {dst.offset[0], dst.offset[1], dst.offset[2]}
        );
    }

    // Vulkan needs these

    auto Transfer_Encoder::persist(opaque::Buffer::View buffer) noexcept -> void {}
    auto Transfer_Encoder::persist(opaque::Image::View image) noexcept -> void {}
    auto Transfer_Encoder::persist(opaque::Grid::View grid) noexcept -> void {}

    auto Transfer_Encoder::acquire(opaque::Buffer::View buffer) noexcept -> void {}
    auto Transfer_Encoder::acquire(opaque::Image::View image) noexcept -> void {}
    auto Transfer_Encoder::acquire(opaque::Grid::View grid) noexcept -> void {}

    auto Transfer_Encoder::release(mut<command::Buffer> dst, opaque::Buffer::View buffer) noexcept -> void {}
    auto Transfer_Encoder::release(mut<command::Buffer> dst, opaque::Image::View image) noexcept -> void {}
    auto Transfer_Encoder::release(mut<command::Buffer> dst, opaque::Grid::View grid) noexcept -> void {}

}
