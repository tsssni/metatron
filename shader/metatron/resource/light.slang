module light;
__include light.area;
__include light.atomosphere;
__include light.environment;
__include light.parallel;
__include light.point;
__include light.spot;

import spectra;
import shape;
import texture;
import metatron.core;

#define DISPATCH_V(method, ...) \
         if (idx.type() == 0) return tag<Parallel_Light>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Point_Light>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 2) return tag<Spot_Light>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 3) return tag<Area_Light>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 4) return tag<Environment_Light>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 5) return tag<Atomosphere_Light>(idx).get().method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

#define DISPATCH_OP(...) \
         if (idx.type() == 0) return tag<Parallel_Light>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Point_Light>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 2) return tag<Spot_Light>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 3) return tag<Area_Light>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 4) return tag<Environment_Light>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 5) return tag<Atomosphere_Light>(idx).get()(__VA_ARGS__); \
    else return {};

namespace mtt::light {
    public struct Interaction {
        public fv4 L;
        public fv3 wi;
        public fv3 p;
        public f32 t;
        public f32 pdf;
    }

    public enum Flags {
        delta = 1 << 0,
        inf = 1 << 1,
    }

    public struct Light {
        tag<byte> idx;
        public Optional<Interaction> operator()(math::Ray r, fv4 lambda) { DISPATCH_OP(r, lambda) }
        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> { DISPATCH_R(sample, ctx, u) }
        public func flags() -> Flags { DISPATCH_R(flags) }
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
#undef DISPATCH_OP
