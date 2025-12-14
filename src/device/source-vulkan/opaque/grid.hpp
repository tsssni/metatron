#pragma once
#include "buffer.hpp"
#include <metatron/device/opaque/grid.hpp>

namespace mtt::opaque {
    struct Grid::Impl final {
        Barrier barrier;
        vk::UniqueDeviceMemory memory;
        vk::UniqueImage image;

        auto static subresource(Grid::View view) noexcept -> vk::ImageSubresource2;
        auto static layers(Grid::View view) noexcept -> vk::ImageSubresourceLayers;
        auto static range(Grid::View view) noexcept -> vk::ImageSubresourceRange;
        auto static offset(Grid::View view) noexcept -> vk::Offset3D;
        auto static extent(Grid::View view) noexcept -> vk::Extent3D;
        auto update(cref<Barrier> desc) noexcept -> vk::ImageMemoryBarrier2;
    };
}
