#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/image.hpp>

namespace mtt::opaque {
    struct Image::Impl final {
        vk::UniqueImage image;
        vk::ImageLayout layout = vk::ImageLayout::eUndefined;
        vk::AccessFlags2 access = vk::AccessFlagBits2::eNone;

        auto format(cref<muldim::Image> image) noexcept -> vk::Format;
    };
}
