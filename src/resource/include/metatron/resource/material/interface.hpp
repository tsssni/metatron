#pragma once
#include <metatron/resource/material/material.hpp>

namespace mtt::material {
    struct Interface_Material final {
        u32 padding = 0u;

        auto sample(
            cref<math::Context> ctx,
            cref<muldim::Coordinate> coord
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
