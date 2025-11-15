#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/shape/shape.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::light {
    struct Area_Light final {
        tag<shape::Shape> shape;
        usize primitive;

        auto operator()(
            cref<math::Ray> r, cref<fv4> lambda
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<eval::Context> ctx, cref<fv2> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
