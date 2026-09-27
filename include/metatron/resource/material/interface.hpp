#pragma once
#include <metatron/resource/material/interaction.hpp>

namespace mtt::material {
    struct Interface_Material final {
        struct Descriptor final {};
        Interface_Material(Descriptor const&) noexcept;
        Interface_Material() noexcept = default;

        auto sample(
            math::Context const& ctx,
            muldim::Coordinate const& coord
        ) const noexcept -> Interaction<bsdf::Interface_Bsdf>;
        auto flags() const noexcept -> Flags;

    private:
        u32 padding = 0u;
    };
}
