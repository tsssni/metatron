#pragma once
#include <metatron/resource/shape/mesh.hpp>
#include <metatron/resource/shape/sphere.hpp>

namespace mtt::shape {
    auto init() noexcept -> void;

    struct Shape final: stl::polymorph<Shape
    , Mesh
    , Sphere> {
        using polymorph::polymorph;

        auto size() const noexcept -> usize {
            return visit([&](auto* p) noexcept { return p->size(); });
        }
        auto bounding_box(math::Transform const& t, usize idx) const noexcept -> math::Bounding_Box {
            return visit([&, idx](auto* p) noexcept { return p->bounding_box(t, idx); });
        }
        auto operator()(
            math::Ray const& r, fv3 const& np,
            fv4 const& pos, usize idx
        ) const noexcept -> Interaction {
            return visit([&, idx](auto* p) noexcept { return (*p)(r, np, pos, idx); });
        }
        auto sample(math::Context const& ctx, fv2 const& u, usize idx) const noexcept -> Interaction {
            return visit([&, idx](auto* p) noexcept { return p->sample(ctx, u, idx); });
        }
        auto query(math::Ray const& r, usize idx) const noexcept -> fv4 {
            return visit([&, idx](auto* p) noexcept { return p->query(r, idx); });
        }
    };

    template<typename T>
    concept Intersectable = Shape::ts::template contains<T>;
}
