#pragma once
#include <metatron/resource/material/material.hpp>

namespace mtt::material {
    struct Interface_Material final {
        auto sample(
            cref<eval::Context> ctx,
            cref<image::Coordinate> coord
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
