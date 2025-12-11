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

    Image::Image(cref<Descriptor> desc) noexcept:
    type(desc.type),
    state(desc.state) {
        width = desc.image->width;
        height = desc.image->height;
        mips = math::max(1uz, desc.image->pixels.size());
        if (desc.state == State::samplable && !desc.image->pixels.empty()) {
            for (auto i = 0; i < mips; ++i)
                host.push_back(make_obj<Buffer>(Buffer::Descriptor{
                    .type = type,
                    .state = Buffer::State::visible,
                    .ptr = desc.image->pixels[i].data(),
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
        auto& queue = command::Queues::instance().queues[u32(type)];
        auto device = ctx->device.get();
        command::guard(device.createImageUnique({
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = vk::Extent3D{width, height, 1},
            .mipLevels = mips,
            .arrayLayers = 1,
            .usage = usages,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue.family,
        }));
    }
}
