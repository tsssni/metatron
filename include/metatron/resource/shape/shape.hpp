#pragma once
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/shape/sphere.hpp>

namespace mtt::shape {
    struct Shape final: stl::polynomial<Shape
    , Mesh
    , Sphere> {
        using polynomial::polynomial;
        auto static init() noexcept -> void;

        auto size() const noexcept -> usize {
            return visit([&](auto* p) noexcept { return p->size(); });
        }
        auto bounding_box(cref<math::Transform> t, usize idx) const noexcept -> math::Bounding_Box {
            return visit([&, idx](auto* p) noexcept { return p->bounding_box(t, idx); });
        }
        auto operator()(
            cref<math::Ray> r, cref<fv3> np,
            cref<fv4> pos, usize idx
        ) const noexcept -> opt<Interaction> {
            return visit([&, idx](auto* p) noexcept { return (*p)(r, np, pos, idx); });
        }
        auto sample(cref<math::Context> ctx, cref<fv2> u, usize idx) const noexcept -> opt<Interaction> {
            return visit([&, idx](auto* p) noexcept { return p->sample(ctx, u, idx); });
        }
        auto query(cref<math::Ray> r, usize idx) const noexcept -> opt<fv4> {
            return visit([&, idx](auto* p) noexcept { return p->query(r, idx); });
        }
    };
}
