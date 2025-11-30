#pragma once
#include <metatron/resource/material/material.hpp>

namespace mtt::material {
    struct Interface_Material final {
        auto sample(
            cref<math::Context> ctx,
            cref<opaque::Coordinate> coord
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
