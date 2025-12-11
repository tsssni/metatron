#include "grid.hpp"

namespace mtt::opaque {
    Grid::Grid(cref<Descriptor> desc) noexcept:
    type(desc.type),
    state(desc.state) {
        width = desc.grid->width;
        height = desc.grid->height;
        depth = desc.grid->depth;
        if (desc.state == State::readonly && !desc.grid->cells.empty()) {
            host = make_obj<Buffer>(Buffer::Descriptor{
                .type = type,
                .state = Buffer::State::visible,
                .ptr = mut<byte>(desc.grid->cells.data()),
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
        auto& queue = command::Queues::instance().queues[u32(type)];
        auto device = ctx->device.get();
        command::guard(device.createImageUnique({
            .imageType = vk::ImageType::e3D,
            .format = vk::Format::eR32Sfloat,
            .extent = vk::Extent3D{width, height, depth},
            .mipLevels = 1,
            .arrayLayers = 1,
            .usage = usages,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &queue.family,
        }));
    }
}
