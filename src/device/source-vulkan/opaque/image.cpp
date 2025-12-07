#include "image.hpp"

namespace mtt::opaque {
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

    Image::Image(muldim::Image&& image) noexcept {
        width = image.width;
        height = image.height;
        mips = image.pixels.size();
        host = std::move(image.pixels);

        auto type = command::Queue::Type::transfer;
        auto& ctx = command::Context::instance().impl;
        auto& queue = command::Queues::instance().queues[u32(type)];
        auto device = ctx->device.get();
        command::guard(device.createImageUnique({
            .imageType = vk::ImageType::e2D,
            .format = impl->format(image),
            .extent = vk::Extent3D{width, height, 1},
            .mipLevels = mips,
            .arrayLayers = 1,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue.family,
        }));
    }
}
