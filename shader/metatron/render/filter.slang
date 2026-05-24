module filter;
__include filter.box;
__include filter.gaussian;
__include filter.lanczos;
import metatron.core;

#define DISPATCH_V(method, ...) \
         if (idx.type() == 0) return tag<Box_Filter>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Gaussian_Filter>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 2) return tag<Lanczos_Filter>(idx).get().method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

#define DISPATCH_OP(...) \
         if (idx.type() == 0) return tag<Box_Filter>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Gaussian_Filter>(idx).get()(__VA_ARGS__); \
    else if (idx.type() == 2) return tag<Lanczos_Filter>(idx).get()(__VA_ARGS__); \
    else return {};

namespace mtt::filter {
    public struct Interaction {
        public fv2 p;
        public f32 weight;
        public f32 pdf;
    }

    public struct Filter {
        tag<byte> idx;
        public f32 operator()(fv2 p) { DISPATCH_OP(p) }
        public func sample(fv2 u) -> Optional<Interaction> { DISPATCH_R(sample, u) }
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
#undef DISPATCH_OP
