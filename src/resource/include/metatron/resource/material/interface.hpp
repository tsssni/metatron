#pragma once
#include <metatron/resource/material/material.hpp>

namespace mtt::material {
    struct Interface_Material final {
        struct Descriptor final {};
        Interface_Material(cref<Descriptor>) noexcept;
        Interface_Material() noexcept = default;

        auto sample(
            cref<math::Context> ctx,
            cref<muldim::Coordinate> coord
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;

    private:
        u32 padding = 0u;
    };
}
