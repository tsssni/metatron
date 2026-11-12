#pragma once
#include <metatron/render/accel/interaction.hpp>
#include <vector>

namespace mtt::render {

struct [[mtt::reflection]] LBVH final {
    struct Index final {
        core::Bounding_Box bbox;
        union {
            u32 prim;
            u32 right;
        };
        union {
            i32 num_prims;
            u32 axis;
        };
    };

    buf<Index> tlas;
    std::vector<buf<Index>> blas;
    buf<u32> instances;
    buf<Divider const*> dividers;
    std::vector<buf<u32>> prims;

    LBVH(std::span<Divider const* const> dividers, u32 num_guide_leaf_prims) noexcept;

    auto operator()(
        core::Ray const& r, fv3 const& n,
        Accel_Flags flags, fv2 const& range
    ) const noexcept -> Accel_Interaction;

private:
    auto build(
        std::vector<core::Bounding_Box> const& boxes,
        u32 num_guide_leaf_prims
    ) noexcept -> std::tuple<std::vector<u32>, std::vector<Index>>;

    auto traverse(
        Divider const* div, Accel_Interaction& intr,
        core::Ray const& r, Accel_Flags flags, fv2 const& range, u32 k = 0
    ) const noexcept -> bool;
};

}
