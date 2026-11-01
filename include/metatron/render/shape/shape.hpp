#pragma once
#include <metatron/render/shape/mesh.hpp>
#include <metatron/render/shape/sphere.hpp>

namespace mtt::render {

struct Shape final : core::protocol<Mesh, Sphere> {
    using protocol::protocol;

    auto size() const noexcept -> usize;
    auto bounding_box(core::Transform const& t, usize idx) const noexcept -> core::Bounding_Box;
    auto operator()(
        core::Ray const& r, fv3 const& np,
        fv4 const& pos, usize idx
    ) const noexcept -> Shape_Interaction;
    auto sample(core::Ray_Context const& ctx, fv2 const& u, usize idx) const noexcept -> Shape_Interaction;
    auto query(core::Ray const& r, usize idx) const noexcept -> fv4;
};

auto inline Shape::size() const noexcept -> usize {
    return visit([&](auto* p) noexcept { return p->size(); });
}

auto inline Shape::bounding_box(core::Transform const& t, usize idx) const noexcept -> core::Bounding_Box {
    return visit([&, idx](auto* p) noexcept { return p->bounding_box(t, idx); });
}

auto inline Shape::operator()(
    core::Ray const& r, fv3 const& np,
    fv4 const& pos, usize idx
) const noexcept -> Shape_Interaction {
    return visit([&, idx](auto* p) noexcept { return (*p)(r, np, pos, idx); });
}

auto inline Shape::sample(core::Ray_Context const& ctx, fv2 const& u, usize idx) const noexcept -> Shape_Interaction {
    return visit([&, idx](auto* p) noexcept { return p->sample(ctx, u, idx); });
}

auto inline Shape::query(core::Ray const& r, usize idx) const noexcept -> fv4 {
    return visit([&, idx](auto* p) noexcept { return p->query(r, idx); });
}

template<typename T>
concept Intersectable = Shape::ts::template contains<T>;

}
