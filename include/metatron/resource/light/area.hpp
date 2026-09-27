#pragma once
#include <metatron/resource/light/interaction.hpp>
#include <metatron/resource/shape/shape.hpp>

namespace mtt::light {
    struct Area_Light final {
        shape::Shape shape;
        u32 primitive;

        auto operator()(
            math::Ray const& r, fv4 const& lambda
        ) const noexcept -> Interaction;
        auto sample(
            math::Context const& ctx, fv2 const& u
        ) const noexcept -> Interaction;
        auto flags() const noexcept -> Flags;
    };
}
