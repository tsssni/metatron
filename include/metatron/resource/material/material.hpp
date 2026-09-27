#pragma once
#include <metatron/resource/material/physical.hpp>
#include <metatron/resource/material/interface.hpp>

namespace mtt::material {
    auto init() noexcept -> void;

    struct Material final: stl::polynomial<Material
    , Physical_Material
    , Interface_Material> {
        using polynomial::polynomial;

        auto sample(
            cref<math::Context> ctx,
            cref<muldim::Coordinate> coord
        ) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return p->sample(ctx, coord); });
        }
        auto flags() const noexcept -> Flags {
            return visit([&](auto* p) noexcept { return p->flags(); });
        }
    };
}
