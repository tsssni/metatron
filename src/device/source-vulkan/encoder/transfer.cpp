#include "transfer.hpp"
#include "../command/buffer.hpp"
#include "../opaque/buffer.hpp"
#include "../opaque/image.hpp"
#include "../opaque/grid.hpp"

namespace mtt::encoder {
    using Buffer = opaque::Buffer;
    using Image = opaque::Image;
    using Grid = opaque::Grid;
    using Barrier = opaque::Barrier;

    Transfer_Encoder::Transfer_Encoder(mut<command::Buffer> cmd) noexcept {
        this->cmd = cmd;
        impl->src_barrier = {
            .stage = vk::PipelineStageFlagBits2::eCopy,
            .access = vk::AccessFlagBits2::eTransferRead,
            .layout = vk::ImageLayout::eTransferSrcOptimal,
            .family = cmd->impl->family,
        };
        impl->dst_barrier = {
            .stage = vk::PipelineStageFlagBits2::eCopy,
            .access = vk::AccessFlagBits2::eTransferWrite,
            .layout = vk::ImageLayout::eTransferDstOptimal,
            .family = cmd->impl->family,
        };
    }

    auto Transfer_Encoder::upload(Buffer::View view) noexcept -> void {
        using State = Buffer::State;
        auto buffer = view.ptr;
        if (buffer->state == State::local && buffer->ptr) {
            auto uploaded = make_obj<Buffer>();
            auto buffer = view.ptr;
            uploaded->impl->host_memory = std::move(buffer->impl->host_memory);
            uploaded->impl->host_buffer = std::move(buffer->impl->host_buffer);
            uploaded->type = buffer->type;
            uploaded->state = Buffer::State::visible;
            uploaded->ptr = buffer->ptr;
            uploaded->size = buffer->size;

            auto& ctx = command::Context::instance().impl;
            auto device = ctx->device.get();
            auto usages = vk::BufferUsageFlags2CreateInfo{.usage = buffer->impl->usages};

            auto create = vk::BufferCreateInfo{
                .pNext = &usages,
                .size = uploaded->size,
                .sharingMode = vk::SharingMode::eExclusive,
                .queueFamilyIndexCount = 1,
                .pQueueFamilyIndices = &cmd->impl->family,
            };
            uploaded->impl->device_buffer = command::guard(device.createBufferUnique(create));

            auto info = vk::BindBufferMemoryInfo{
                .buffer = uploaded->impl->device_buffer.get(),
                .memory = uploaded->impl->host_memory.get(),
                .memoryOffset = 0,
            };
            command::guard(device.bindBufferMemory2(1, &info));
            uploaded->addr = device.getBufferAddress({
                .buffer = uploaded->impl->device_buffer.get(),
            });
            buffer->ptr = nullptr;

            auto staged = Buffer::View(*uploaded);
            cmd->stages.push_back(std::move(uploaded));
            copy(view, staged);
        } else if (buffer->state == State::twin && !buffer->dirty.empty()) {
            auto buffer = view.ptr;
            auto dirty = std::move(buffer->dirty);
            auto sum = std::ranges::fold_left(dirty, 0, [](auto&& x, auto&& y) {
                return x + y[1];
            });
            auto block = cmd->blocks.allocate(sum);
            auto regions = std::vector<vk::BufferCopy2>{};
            for (auto [offset, size]: dirty) {
                auto dst = block.ptr->ptr + block.offset;
                auto src = buffer->ptr + offset;
                std::memcpy(dst, src, size);
                regions.push_back({
                    .srcOffset = block.offset,
                    .dstOffset = offset,
                    .size = size,
                });
                block.offset += size;
            }

            auto cmd = this->cmd->impl->cmd.get();
            auto barriers = std::to_array<vk::BufferMemoryBarrier2>({
                block.ptr->impl->update(impl->src_barrier),
                buffer->impl->update(impl->dst_barrier),
            });
            cmd.pipelineBarrier2({
                .bufferMemoryBarrierCount = barriers.size(),
                .pBufferMemoryBarriers = barriers.data(),
            });
            cmd.copyBuffer2({
                .srcBuffer = block.ptr->impl->device_buffer.get(),
                .dstBuffer = buffer->impl->device_buffer.get(),
                .regionCount = u32(regions.size()),
                .pRegions = regions.data(),
            });
        }
    }

    auto Transfer_Encoder::upload(Image::View view) noexcept -> void {
        using State = Image::State;
        auto image = view.ptr;
        if (image->state != State::samplable || image->host.empty()) return;
        for (auto i = 0; i < image->host.size(); ++i) {
            auto mip = view;
            mip.mip = {i, 1};
            mip.size = math::foreach([i](auto x, auto) {
                return math::max(x >> i, 1uz);
            }, view.size);
            copy(mip, *image->host[i]);
            cmd->stages.push_back(std::move(image->host[i]));
        }
        image->host.clear();
    }

    auto Transfer_Encoder::upload(Grid::View view) noexcept -> void {
        using State = Grid::State;
        auto grid = view.ptr;
        if (grid->state != State::readonly || !grid->host) return;
        copy(view, *grid->host);
        cmd->stages.push_back(std::move(grid->host));
    }

    template<typename T>
    auto Transfer_Encoder::Impl::transfer(mut<Transfer_Encoder> encoder, u32 family, T view) noexcept -> void {
        auto cmd = encoder->cmd->impl->cmd.get();
        auto transfer = view.ptr->impl->barrier;
        transfer.family = family;
        auto barrier = view.ptr->impl->update(transfer);
        if constexpr (std::is_same_v<T, Buffer::View>)
            cmd.pipelineBarrier2({
                .bufferMemoryBarrierCount = 1,
                .pBufferMemoryBarriers = &barrier,
            });
        else
            cmd.pipelineBarrier2({
                .imageMemoryBarrierCount = 1,
                .pImageMemoryBarriers = &barrier,
            });
    };

    auto Transfer_Encoder::acquire(opaque::Buffer::View buffer) noexcept -> void { impl->transfer(this, cmd->impl->family, buffer); }
    auto Transfer_Encoder::acquire(opaque::Image::View image) noexcept -> void { impl->transfer(this, cmd->impl->family, image); }
    auto Transfer_Encoder::acquire(opaque::Grid::View grid) noexcept -> void { impl->transfer(this, cmd->impl->family, grid); }
    auto Transfer_Encoder::release(mut<command::Buffer> dst, opaque::Buffer::View buffer) noexcept -> void { impl->transfer(this, dst->impl->family, buffer); }
    auto Transfer_Encoder::release(mut<command::Buffer> dst, opaque::Image::View image) noexcept -> void { impl->transfer(this, dst->impl->family, image); }
    auto Transfer_Encoder::release(mut<command::Buffer> dst, opaque::Grid::View grid) noexcept -> void { impl->transfer(this, dst->impl->family, grid); }

    auto Transfer_Encoder::copy(Buffer::View dst, Buffer::View src) noexcept -> void {
        auto cmd = this->cmd->impl->cmd.get();
        auto barriers = std::to_array<vk::BufferMemoryBarrier2>({
            src.ptr->impl->update(impl->src_barrier),
            dst.ptr->impl->update(impl->dst_barrier),
        });
        cmd.pipelineBarrier2({
            .bufferMemoryBarrierCount = barriers.size(),
            .pBufferMemoryBarriers = barriers.data(),
        });

        auto region = vk::BufferCopy2{
            .srcOffset = src.offset,
            .dstOffset = dst.offset,
            .size = math::min(dst.size, src.size),
        };
        cmd.copyBuffer2({
            .srcBuffer = src.ptr->impl->device_buffer.get(),
            .dstBuffer = dst.ptr->impl->device_buffer.get(),
            .regionCount = 1,
            .pRegions = &region,
        });
    }

    template<typename T, typename U>
    auto Transfer_Encoder::Impl::copy(mut<Transfer_Encoder> encoder, T to, U from) noexcept -> void {
        auto cmd = encoder->cmd->impl->cmd.get();
        auto& src = from.ptr->impl;
        auto& dst = to.ptr->impl;
        auto src_barrier = src->update(this->src_barrier);
        auto dst_barrier = dst->update(this->dst_barrier);
        auto barrier_info = vk::DependencyInfo{.bufferMemoryBarrierCount = 1, .imageMemoryBarrierCount = 1};
        auto region_info = vk::BufferImageCopy2{.bufferRowLength = 0, .bufferImageHeight = 0};
        if constexpr (std::is_same_v<U, Buffer::View>) {
            barrier_info.pBufferMemoryBarriers = &src_barrier;
            barrier_info.pImageMemoryBarriers = &dst_barrier;
            cmd.pipelineBarrier2(barrier_info);

            region_info.bufferOffset = from.offset;
            region_info.imageSubresource = to.ptr->impl->layers(to);
            region_info.imageOffset = to.ptr->impl->offset(to);
            region_info.imageExtent = to.ptr->impl->extent(to);
            cmd.copyBufferToImage2({
                .srcBuffer = src->device_buffer.get(),
                .dstImage = dst->image.get(),
                .dstImageLayout = dst->barrier.layout,
                .regionCount = 1,
                .pRegions = &region_info,
            });
        } else {
            barrier_info.pBufferMemoryBarriers = &dst_barrier;
            barrier_info.pImageMemoryBarriers = &src_barrier;
            cmd.pipelineBarrier2(barrier_info);

            region_info.bufferOffset = to.offset;
            region_info.imageSubresource = from.ptr->impl->layers(from);
            region_info.imageOffset = from.ptr->impl->offset(from);
            region_info.imageExtent = from.ptr->impl->extent(from);
            cmd.copyImageToBuffer2({
                .srcImage = src->image.get(),
                .srcImageLayout = src->barrier.layout,
                .dstBuffer = dst->device_buffer.get(),
                .regionCount = 1,
                .pRegions = &region_info,
            });
        }
    }

    auto Transfer_Encoder::copy(Image::View dst, Buffer::View src) noexcept -> void { impl->copy(this, dst, src); }
    auto Transfer_Encoder::copy(Grid::View dst, Buffer::View src) noexcept -> void { impl->copy(this, dst, src); }
    auto Transfer_Encoder::copy(Buffer::View dst, Image::View src) noexcept -> void { impl->copy(this, dst, src); }
    auto Transfer_Encoder::copy(Buffer::View dst, Grid::View src) noexcept -> void { impl->copy(this, dst, src); }

    template<typename T>
    auto Transfer_Encoder::Impl::copy(mut<Transfer_Encoder> encoder, T to, T from) noexcept -> void {
        auto cmd = encoder->cmd->impl->cmd.get();
        auto& src = from.ptr->impl;
        auto& dst = to.ptr->impl;
        auto barriers = std::to_array<vk::ImageMemoryBarrier2>({
            src->update(src_barrier),
            dst->update(dst_barrier),
        });
        auto barrier_info = vk::DependencyInfo{
            .imageMemoryBarrierCount = 2,
            .pImageMemoryBarriers = barriers.data(),
        };
        cmd.pipelineBarrier2(barrier_info);

        auto region_info = vk::ImageCopy2{
            .srcSubresource = from.ptr->impl->layers(from),
            .srcOffset = from.ptr->impl->offset(from),
            .dstSubresource = from.ptr->impl->layers(to),
            .dstOffset = from.ptr->impl->offset(to),
            .extent = from.ptr->impl->extent(to),
        };
        cmd.copyImage2({
            .srcImage = src->image.get(),
            .srcImageLayout = src->barrier.layout,
            .dstImage = dst->image.get(),
            .dstImageLayout = dst->barrier.layout,
            .regionCount = 1,
            .pRegions = &region_info,
        });
    }

    auto Transfer_Encoder::copy(Image::View dst, Image::View src) noexcept -> void { impl->copy(this, src, dst); }
    auto Transfer_Encoder::copy(Grid::View dst, Grid::View src) noexcept -> void { impl->copy(this, src, dst); }
}
