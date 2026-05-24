module phase;
__include phase.henyey_greenstein;
import metatron.core;

#define DISPATCH_V(method, ...) \
    if (idx == 0) return henyey_greenstein.method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

#define DISPATCH_OP(...) \
    if (idx == 0) return henyey_greenstein(__VA_ARGS__); \
    else return {};

namespace mtt::phase {
    public struct Interaction {
        public fv4 f;
        public fv3 wi;
        public f32 pdf;
    }

    public struct Phase_Function {
        Henyey_Greenstein_Phase_Function henyey_greenstein = {};
        u32 idx = 0xffffffffu;
        public __init() {}
        public __init(Henyey_Greenstein_Phase_Function henyey_greenstein) {
            this.henyey_greenstein = henyey_greenstein; idx = 0;
        }

        public Optional<Interaction> operator()(fv3 wo, fv3 wi) { DISPATCH_OP(wo, wi) }
        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> { DISPATCH_R(sample, ctx, u) }
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
#undef DISPATCH_OP
