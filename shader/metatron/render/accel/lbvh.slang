implementing accel;

namespace mtt::accel {
    // TODO:
    // support software acceleration structure in the future.
    // now just occupy the space in variant.

    public struct LBVH: Acceleration {
        struct Primitive {
            math::Bounding_Box bbox;
            tag<Divider> instance;
            u32 primitive;
            u32 morton_code;
        };

        // TODO: replace union on cpu
        struct Index {
            math::Bounding_Box bbox;
            u32 prim;
            u32 num_prims;
        };

        public Optional<Interaction> operator()(math::Ray r, fv3 n) { return none; }

        buf<Primitive> prims = {};
        buf<Index> bvh = {};
    }
}
