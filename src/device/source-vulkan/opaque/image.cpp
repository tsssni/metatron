#include "image.hpp"
#include "../command/buffer.hpp"

namespace mtt::opaque {
    auto Image::Impl::subresource(Image::View view) noexcept -> vk::ImageSubresource2 {
        return {.imageSubresource = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = u32(view.mip[0]),
            .arrayLayer = 0,
        }};
    }

    auto Image::Impl::layers(Image::View view) noexcept -> vk::ImageSubresourceLayers {
        return {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = u32(view.mip[0]),
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
    }

    auto Image::Impl::range(Image::View view) noexcept -> vk::ImageSubresourceRange {
        return {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = u32(view.mip[0]),
            .levelCount = u32(view.mip[1]),
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
    }

    auto Image::Impl::offset(Image::View view) noexcept -> vk::Offset3D {
        return {i32(view.offset[0]), i32(view.offset[1]), 0};
    }

    auto Image::Impl::extent(Image::View view) noexcept -> vk::Extent3D {
        return {u32(view.size[0]), u32(view.size[1]), 1};
    }

    auto Image::Impl::format(cref<muldim::Image> image) noexcept -> vk::Format {
        auto channels = image.channels;
        auto stride = image.stride;
        auto linear = image.linear;

        if (stride == 1) {
            if (channels == 1) {
                if (linear) return vk::Format::eR8Unorm;
                else return vk::Format::eR8Srgb;
            } else if (channels == 2) {
                if (linear) return vk::Format::eR8G8Unorm;
                else return vk::Format::eR8G8Srgb;
            } else if (channels == 4) {
                if (linear) return vk::Format::eR8G8B8A8Unorm;
                else return vk::Format::eR8G8B8A8Srgb;
            }
        } else if (stride == 4) {
            if (channels == 1) return vk::Format::eR32Sfloat;
            else if (channels == 2) return vk::Format::eR32G32Sfloat;
            else if (channels == 4) return vk::Format::eR32G32B32A32Sfloat;
        }

        stl::abort(
            "image not supported in vulkan with channels {} stride {} linear {}",
            channels, stride, linear
        );
        return vk::Format::eB8G8R8A8Unorm;
    }

    auto Image::Impl::update(cref<Barrier> desc) noexcept -> vk::ImageMemoryBarrier2 {
        auto barrier = this->barrier.update<vk::ImageMemoryBarrier2>(desc);
        barrier.image = image.get();
        barrier.subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = vk::RemainingMipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        return barrier;
    }

    Image::Image(cref<Descriptor> desc) noexcept:
    state(desc.state),
    type(desc.cmd->type) {
        impl->barrier.family = desc.cmd->impl->family;
        width = desc.image->width;
        height = desc.image->height;
        mips = math::max(1uz, desc.image->pixels.size());
        if (desc.state == State::samplable && !desc.image->pixels.empty()) {
            for (auto i = 0; i < mips; ++i)
                host.push_back(make_obj<Buffer>(Buffer::Descriptor{
                    .cmd = desc.cmd,
                    .ptr = desc.image->pixels[i].data(),
                    .state = Buffer::State::visible,
                    .size = desc.image->pixels[i].size(),
                }));
            desc.image->pixels.clear();
        }

        auto format = impl->format(*desc.image);
        auto usages = vk::ImageUsageFlags{}
        | vk::ImageUsageFlagBits::eTransferSrc
        | vk::ImageUsageFlagBits::eTransferDst;
        switch (state) {
        case State::samplable: usages |= vk::ImageUsageFlagBits::eSampled; break;
        case State::storable: usages |= vk::ImageUsageFlagBits::eStorage; break;
        default: break;
        }

        auto& ctx = command::Context::instance().impl;
        auto device = ctx->device.get();
        impl->image = command::guard(device.createImageUnique({
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = vk::Extent3D{width, height, 1},
            .mipLevels = mips,
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

    Image::operator View() noexcept {
        return {this, {0, mips}, {0, 0}, {width, height}};
    }
}
