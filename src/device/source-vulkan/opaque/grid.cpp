#include "grid.hpp"
#include "../command/buffer.hpp"

namespace mtt::opaque {
    auto Grid::Impl::subresource(Grid::View view) noexcept -> vk::ImageSubresource2 {
        return {.imageSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .arrayLayer = 0,
        }};
    }

    auto Grid::Impl::layers(Grid::View view) noexcept -> vk::ImageSubresourceLayers {
        return {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
    }

    auto Grid::Impl::range(Grid::View view) noexcept -> vk::ImageSubresourceRange {
        return {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
    }

    auto Grid::Impl::offset(Grid::View view) noexcept -> vk::Offset3D {
        return {i32(view.offset[0]), i32(view.offset[1]), i32(view.offset[2])};
    }

    auto Grid::Impl::extent(Grid::View view) noexcept -> vk::Extent3D {
        return {u32(view.size[0]), u32(view.size[1]), u32(view.size[2])};
    }

    auto Grid::Impl::update(cref<Barrier> desc) noexcept -> vk::ImageMemoryBarrier2 {
        auto barrier = this->barrier.update<vk::ImageMemoryBarrier2>(desc);
        barrier.image = image.get();
        barrier.subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 0,
        };
        return barrier;
    }

    Grid::Grid(cref<Descriptor> desc) noexcept:
    state(desc.state),
    type(desc.cmd->type) {
        impl->barrier.family = desc.cmd->impl->family;
        width = desc.grid->width;
        height = desc.grid->height;
        depth = desc.grid->depth;
        if (desc.state == State::readonly && !desc.grid->cells.empty()) {
            host = make_obj<Buffer>(Buffer::Descriptor{
                .cmd = desc.cmd,
                .ptr = mut<byte>(desc.grid->cells.data()),
                .state = Buffer::State::visible,
                .size = desc.grid->cells.size() * sizeof(f32),
            });
            desc.grid->cells.clear();
        }

        auto usages = vk::ImageUsageFlags{}
        | vk::ImageUsageFlagBits::eTransferSrc
        | vk::ImageUsageFlagBits::eTransferDst;
        switch (state) {
        case State::readonly: usages |= vk::ImageUsageFlagBits::eSampled; break;
        case State::writable: usages |= vk::ImageUsageFlagBits::eStorage; break;
        default: break;
        }

        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        impl->image = command::guard(device.createImageUnique({
            .imageType = vk::ImageType::e3D,
            .format = vk::Format::eR32Sfloat,
            .extent = vk::Extent3D{width, height, depth},
            .mipLevels = 1,
            .arrayLayers = 1,
            .usage = usages,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &impl->barrier.family,
        }));

        auto size = device.getImageMemoryRequirements2({
            .image = impl->image.get(),
        }).memoryRequirements.size;
        auto alloc = vk::MemoryAllocateInfo{
            .allocationSize = size,
            .memoryTypeIndex = ctx->device_memory,
        };
        impl->memory = command::guard(device.allocateMemoryUnique(alloc));

        auto info = vk::BindImageMemoryInfo{
            .image = impl->image.get(),
            .memory = impl->memory.get(),
            .memoryOffset = 0,
        };
        command::guard(device.bindImageMemory2(1, &info));
    }

    Grid::operator View() noexcept {
        return {this, {0}, {width, height, depth}};
    }
}
