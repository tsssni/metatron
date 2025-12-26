#pragma once
#include <metatron/resource/shape/shape.hpp>

namespace mtt::shape {
    struct Sphere final {
        struct Descriptor final {};
        Sphere() noexcept = default;
        Sphere(cref<Descriptor> desc) noexcept;
        
        auto size() const noexcept -> usize;
        auto bounding_box(
            cref<math::Transform> t, usize idx
        ) const noexcept -> math::Bounding_Box;
        auto operator()(
            cref<math::Ray> r, cref<fv3> np,
            cref<fv4> pos, usize idx = 0uz
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u, usize idx = 0uz
        ) const noexcept -> opt<Interaction>;
        auto query(
            cref<math::Ray> r, usize idx = 0uz
        ) const noexcept -> opt<fv4>;
    };
}
