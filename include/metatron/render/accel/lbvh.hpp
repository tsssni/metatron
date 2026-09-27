#pragma once
#include <metatron/render/accel/interaction.hpp>

namespace mtt::accel {
    struct LBVH final {
        struct Primitive final {
            math::Bounding_Box bbox;
            proxy::Divider instance;
            u32 primitive;
            u32 morton_code;
        };

        struct Index final {
            math::Bounding_Box bbox;
            union { u32 prim; u32 right; };
            union { i32 num_prims; u32 axis; };
        };

        struct Descriptor final {u32 num_guide_leaf_prims = 4;};
        LBVH(Descriptor const& desc) noexcept;
        LBVH() noexcept = default;

        auto operator()(
            math::Ray const& r, fv3 const& n
        ) const noexcept -> Interaction;

    private:
        buf<Primitive> prims;
        buf<Index> bvh;
    };
}
