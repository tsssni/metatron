#pragma once
#include "buffer.hpp"
#include <metatron/device/opaque/image.hpp>

namespace mtt::opaque {
    struct Image::Impl final {
        Barrier barrier;
        vk::UniqueDeviceMemory memory;
        vk::UniqueImage image;

        auto static subresource(Image::View view) noexcept -> vk::ImageSubresource2;
        auto static layers(Image::View view) noexcept -> vk::ImageSubresourceLayers;
        auto static range(Image::View view) noexcept -> vk::ImageSubresourceRange;
        auto static offset(Image::View view) noexcept -> vk::Offset3D;
        auto static extent(Image::View view) noexcept -> vk::Extent3D;

        auto format(cref<muldim::Image> image) noexcept -> vk::Format;
        auto update(cref<Barrier> desc) noexcept -> vk::ImageMemoryBarrier2;
    };
}
