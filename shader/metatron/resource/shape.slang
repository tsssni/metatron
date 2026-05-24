module shape;
__include shape.mesh;
__include shape.sphere;
import metatron.core;

#define DISPATCH_V(method, ...) \
         if (idx.type() == 0) return tag<Mesh>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Sphere>(idx).get().method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

#define DISPATCH_OP(...) \
         if (idx.type() == 0) return tag<Mesh>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Sphere>(idx).get()(__VA_ARGS__); \
    else return {};

namespace mtt::shape {
    public struct Interaction {
        public fv3 p;
        public fv3 n;
        public fv3 tn;
        public fv3 bn;
        public fv2 uv;
        public f32 t;
        public f32 pdf;

        public fv3 dpdu;
        public fv3 dpdv;
        public fv3 dndu;
        public fv3 dndv;
    }

    public struct Shape {
        tag<byte> idx;
        public func size() -> u32 { DISPATCH_R(size) }
        public func bounding_box(math::Transform t, u32 i) -> math::Bounding_Box { DISPATCH_R(bounding_box, t, i) }
        public Optional<Interaction> operator()(math::Ray r, fv3 np, fv4 pos, u32 i) { DISPATCH_OP(r, np, pos, i) }
        public func sample(math::Context ctx, fv2 u, u32 i) -> Optional<Interaction> { DISPATCH_R(sample, ctx, u, i) }
        public func query(math::Ray r, u32 i) -> Optional<fv4> { DISPATCH_R(query, r, i) }
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
#undef DISPATCH_OP
