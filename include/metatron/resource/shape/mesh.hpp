#pragma once
#include <metatron/resource/shape/interaction.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::shape {
    struct Mesh final {
        buf<uv3> indices;

        buf<fv3> vertices;
        buf<fv3> normals;
        buf<fv2> uvs;

        buf<fv3> dpdu;
        buf<fv3> dpdv;
        buf<fv3> dndu;
        buf<fv3> dndv;

        struct Descriptor final {
            std::string path;
        };
        Mesh(Descriptor const& desc) noexcept;
        Mesh() noexcept = default;

        auto size() const noexcept -> usize;
        auto bounding_box(
            math::Transform const& t, usize idx
        ) const noexcept -> math::Bounding_Box;
        auto operator()(
            math::Ray const& r, fv3 const& np,
            fv4 const& pos, usize idx
        ) const noexcept -> Interaction;
        // sphere triangle sampling: https://pbr-book.org/4ed/Shapes/Triangle_Meshes
        auto sample(
            math::Context const& ctx, fv2 const& u, usize idx
        ) const noexcept -> Interaction;
        auto query(
            math::Ray const& r, usize idx
        ) const noexcept -> fv4;

    private:
        template<typename T>
        auto blerp(
            buf<T> traits,
            fv3 const& b,
            usize idx
        ) const noexcept -> T {
            if (traits.empty()) return {};
            auto prim = indices[idx];
            return math::blerp(
                math::Vector<T, 3>{
                    traits[prim[0]],
                    traits[prim[1]],
                    traits[prim[2]],
                }, b
            );
        }

        auto pdf(
            math::Ray const& r, fv3 const& np, usize idx
        ) const noexcept -> f32;
    };
}
