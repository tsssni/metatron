#pragma once
#include <metatron/render/accel/interaction.hpp>

namespace mtt::accel {
    struct LBVH final {
        struct Index final {
            math::Bounding_Box bbox;
            union { u32 prim; u32 right; };
            union { i32 num_prims; u32 axis; };
        };

        struct Descriptor final {u32 num_guide_leaf_prims = 4;};
        LBVH(Descriptor const& desc) noexcept;
        LBVH() noexcept = default;

        auto operator()(
            math::Ray const& r, fv3 const& n,
            Flags flags, fv2 const& range
        ) const noexcept -> Interaction;

    private:
        auto build(
            std::vector<math::Bounding_Box> const& boxes,
            u32 num_guide_leaf_prims
        ) noexcept -> std::tuple<std::vector<u32>, std::vector<Index>>;

        auto traverse(
            proxy::Divider div, Interaction& intr,
            math::Ray const& r, Flags flags, fv2 const& range
        ) const noexcept -> bool;

        buf<Index> tlas;
        buf<buf<Index>> blas;
        buf<u32> instances;
        buf<buf<u32>> prims;
    };
}
