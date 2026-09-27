#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/image.hpp>

namespace mtt::opaque {
    struct Image::Impl final {
        mtl<MTL::Texture> texture;
        auto format(muldim::Image const& image) noexcept -> MTL::PixelFormat;
    };
}
