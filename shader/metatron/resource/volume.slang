module volume;
__include volume.nanovdb;
__include volume.uniform;
import metatron.core;

#define DISPATCH_V(method, ...) \
         if (idx.type() == 0) return tag<Uniform_Volume>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get().method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

#define DISPATCH_OP(...) \
         if (idx.type() == 0) return tag<Uniform_Volume>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get()(__VA_ARGS__); \
    else return 0.f;

namespace mtt::volume {
    public struct Volume {
        public tag<byte> idx;

        public func to_local(iv3 ijk) -> fv3 { DISPATCH_R(to_local, ijk) }
        public func to_index(fv3 pos) -> iv3 { DISPATCH_R(to_index, pos) }
        public func dimensions() -> uv3 { DISPATCH_R(dimensions) }
        public func inside(iv3 pos) -> bool { DISPATCH_R(inside, pos) }
        public func inside(fv3 pos) -> bool { DISPATCH_R(inside, pos) }
        public func bounding_box() -> math::Bounding_Box { DISPATCH_R(bounding_box) }
        public func bounding_box(fv3 pos) -> math::Bounding_Box { DISPATCH_R(bounding_box, pos) }
        public func bounding_box(iv3 ijk) -> math::Bounding_Box { DISPATCH_R(bounding_box, ijk) }
        public f32 operator()(fv3 pos) { DISPATCH_OP(pos) }

        public __subscript(iv3 ijk) -> f32 { get {
                 if (idx.type() == 0) return (*tag<Uniform_Volume>(idx).get())[ijk];
            else if (idx.type() == 1) return (*tag<Nanovdb_Volume>(idx).get())[ijk];
            else return {};
        }}
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
#undef DISPATCH_OP
