#pragma once
#include <metatron/resource/light/interaction.hpp>
#include <metatron/resource/shape/shape.hpp>

namespace mtt::light {
    struct Area_Light final {
        shape::Shape shape;
        u32 primitive;

        auto operator()(
            cref<math::Ray> r, cref<fv4> lambda
        ) const noexcept -> Interaction;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> Interaction;
        auto flags() const noexcept -> Flags;
    };
}
