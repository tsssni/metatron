#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/image.hpp>

namespace mtt::opaque {
    struct Image::Impl final {
        mtl<MTL::Texture> texture;
        auto format(cref<muldim::Image> image) noexcept -> MTL::PixelFormat;
    };
}
