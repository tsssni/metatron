#pragma once
#include <metatron/resource/material/physical.hpp>
#include <metatron/resource/material/interface.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::material {
    struct Material final: stl::polynomial<Material, Physical_Material, Interface_Material> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto sample(
            cref<math::Context> ctx,
            cref<muldim::Coordinate> coord
        ) const noexcept -> opt<Interaction> {
            return visit([&](auto* p) noexcept { return p->sample(ctx, coord); });
        }
        auto flags() const noexcept -> Flags {
            return visit([&](auto* p) noexcept { return p->flags(); });
        }
    };
}
