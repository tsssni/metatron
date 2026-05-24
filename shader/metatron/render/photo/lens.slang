implementing photo;

#define DISPATCH_V(method, ...) \
         if (idx.type() == 0) return tag<Pinhole_Lens>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Thin_Lens>(idx).get().method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

namespace mtt::photo {
    public struct Lens {
        tag<byte> idx;
        public func sample(fv2 o, fv2 u) -> Optional<lens::Interaction> { DISPATCH_R(sample, o, u) }
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
