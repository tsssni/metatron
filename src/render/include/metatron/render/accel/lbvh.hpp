#pragma once
#include <metatron/render/accel/accel.hpp>
#include <metatron/core/stl/capsule.hpp>

namespace mtt::accel {
    struct LBVH final {
        struct Primitive final {
            math::Bounding_Box bbox;
            tag<Divider> instance;
            u32 primitive;
            u32 morton_code;
        };

        struct Index final {
            math::Bounding_Box bbox;
            union {
                u32 prim;
                u32 right;
            };
            union {
                i32 num_prims;
                u32 axis;
            };
        };

        struct Descriptor final {usize num_guide_leaf_prims = 4;};
        LBVH(Descriptor const& desc) noexcept;

        auto operator()(
            math::Ray const& r,
            math::Vector<f32, 3> const& n
        ) const noexcept -> std::optional<Interaction>;

    private:
        std::vector<Primitive> prims;
        std::vector<Index> bvh;
    };
}
