#pragma once
#include <metatron/resource/material/material.hpp>

namespace mtt::material {
    struct Interface_Material final {
        auto sample (
            eval::Context const& ctx,
            device::Coordinate const& coord
        ) const noexcept -> std::optional<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
