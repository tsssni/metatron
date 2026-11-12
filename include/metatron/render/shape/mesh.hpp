#pragma once
#include <metatron/render/shape/interaction.hpp>
#include <string_view>

namespace mtt::render {

struct [[mtt::reflection]] Mesh final {
    buf<uv3> indices;

    buf<fv3> vertices;
    buf<fv3> normals;
    buf<fv2> uvs;

    buf<fv3> dpdu;
    buf<fv3> dpdv;
    buf<fv3> dndu;
    buf<fv3> dndv;

    usize num_prims = 0;
    Mesh(std::string_view path) noexcept;

    auto size() const noexcept -> usize;
    auto bounding_box(
        core::Transform const& t, usize idx
    ) const noexcept -> core::Bounding_Box;
    auto operator()(
        core::Ray const& r, fv3 const& np,
        fv4 const& pos, usize idx
    ) const noexcept -> Shape_Interaction;
    // sphere triangle sampling: https://pbr-book.org/4ed/Shapes/Triangle_Meshes
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u, usize idx
    ) const noexcept -> Shape_Interaction;
    auto query(
        core::Ray const& r, usize idx
    ) const noexcept -> fv4;

private:
    template<typename T>
    auto blerp(T const* vs, fv3 const& b, usize idx) const noexcept -> T;
    auto pdf(core::Ray const& r, fv3 const& np, usize idx) const noexcept -> f32;
};

template<typename T>
auto Mesh::blerp(
    T const* vs,
    fv3 const& b,
    usize idx
) const noexcept -> T {
    if (!vs) return {};
    auto prim = indices[idx];
    return core::blerp<T>({vs[prim[0]], vs[prim[1]], vs[prim[2]]}, b);
}

}
