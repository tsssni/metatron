#pragma once
#include "../command/context.hpp"
#include <metatron/device/opaque/grid.hpp>

namespace mtt::opaque {
    struct Grid::Impl final {
        mtl<MTL::Texture> texture;
    };
}
