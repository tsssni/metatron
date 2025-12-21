#pragma once
#include <metatron/render/accel/accel.hpp>
#include <metatron/core/stl/stack.hpp>

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
            union { u32 prim; u32 right; };
            union { i32 num_prims; u32 axis; };
        };

        struct Descriptor final {u32 num_guide_leaf_prims = 4;};
        LBVH(cref<Descriptor> desc) noexcept;

        auto operator()(
            cref<math::Ray> r, cref<fv3> n
        ) const noexcept -> opt<Interaction>;

    private:
        buf<Primitive> prims;
        buf<Index> bvh;
    };
}
