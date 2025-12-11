#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/grid.hpp>

namespace mtt::opaque {
    struct Grid::Impl final {
        vk::UniqueImage image;
        vk::ImageLayout layout = vk::ImageLayout::eUndefined;
        vk::AccessFlags2 access = vk::AccessFlagBits2::eNone;
    };
}
