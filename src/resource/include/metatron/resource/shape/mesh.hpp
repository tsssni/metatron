#pragma once
#include <metatron/resource/shape/shape.hpp>
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
        Mesh() noexcept = default;
        Mesh(cref<Descriptor> desc) noexcept;

        auto size() const noexcept -> usize;
        auto bounding_box(
            cref<fm44> t, usize idx
        ) const noexcept -> math::Bounding_Box;
        auto operator()(
            cref<math::Ray> r, cref<fv3> np, usize idx
        ) const noexcept -> opt<Interaction>;
        // sphere triangle sampling: https://pbr-book.org/4ed/Shapes/Triangle_Meshes
        auto sample(
            cref<math::Context> ctx, cref<fv2> u, usize idx
        ) const noexcept -> opt<Interaction>;
        auto query(
            cref<math::Ray> r, usize idx
        ) const noexcept -> opt<f32>;

    private:
        template<typename T>
        auto blerp(
            buf<T> traits,
            cref<fv3> b,
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

        auto intersect(
            cref<math::Ray> r, usize idx
        ) const noexcept -> opt<fv4>;

        auto pdf(
            cref<math::Ray> r, cref<fv3> np, usize idx
        ) const noexcept -> f32;
    };
}
