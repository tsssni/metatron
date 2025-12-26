#include "grid.hpp"
#include "../command/queue.hpp"

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
        return {view.size[0], view.size[1], view.size[2]};
    }

    auto Grid::Impl::update(cref<Barrier> desc) noexcept -> vk::ImageMemoryBarrier2 {
        auto barrier = this->barrier.update<vk::ImageMemoryBarrier2>(desc);
        barrier.image = image.get();
        barrier.subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        return barrier;
    }

    Grid::Grid(cref<Descriptor> desc) noexcept:
    state(desc.state) {
        impl->barrier.family = command::Queue::Impl::family[u32(desc.type)];
        width = desc.grid->width;
        height = desc.grid->height;
        depth = desc.grid->depth;
        if (desc.state == State::readonly && !desc.grid->cells.empty()) {
            host = make_obj<Buffer>(Buffer::Descriptor{
                .ptr = mut<byte>(desc.grid->cells.data()),
                .state = Buffer::State::visible,
                .type = desc.type,
                .size = desc.grid->cells.size() * sizeof(f32),
            });
            desc.grid->cells.clear();
        }

        auto format = vk::Format::eR32Sfloat;
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
            .format = format,
            .extent = Impl::extent(*this),
            .mipLevels = 1,
            .arrayLayers = 1,
            .usage = usages,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &impl->barrier.family,
        }));

        auto reqs = device.getImageMemoryRequirements2({
            .image = impl->image.get(),
        }).memoryRequirements;
        auto alloc = vk::MemoryAllocateInfo{
            .allocationSize = reqs.size,
            .memoryTypeIndex = Buffer::Impl::search(
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                ctx->device_heap, reqs.memoryTypeBits
            ),
        };
        impl->memory = command::guard(device.allocateMemoryUnique(alloc));

        auto info = vk::BindImageMemoryInfo{
            .image = impl->image.get(),
            .memory = impl->memory.get(),
            .memoryOffset = 0,
        };
        command::guard(device.bindImageMemory2(1, &info));

        impl->view = command::guard(device.createImageViewUnique({
            .image = impl->image.get(),
            .viewType = vk::ImageViewType::e3D,
            .format = format,
            .subresourceRange = Impl::range(*this),
        }));
    }

    Grid::operator View() noexcept {
        return {this, {0}, {width, height, depth}};
    }
}
