module phase;
__include phase.henyey_greenstein;
import metatron.core;

namespace mtt::phase {
    public struct Interaction {
        public fv4 f;
        public fv3 wi;
        public f32 pdf;
    }

    public interface Phase_Function {
        public Optional<Interaction> operator()(fv3 wo, fv3 wi);
        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction>;
    }

    public struct Phase_Function_Union: Phase_Function {
        Henyey_Greenstein_Phase_Function henyey_greenstein = {};
        u32 idx = 0xffffffffu;
        public __init() {}
        public __init(Henyey_Greenstein_Phase_Function henyey_greenstein) {
            this.henyey_greenstein = henyey_greenstein; idx = 0;
        }

        public Optional<Interaction> operator()(fv3 wo, fv3 wi) {
            if (idx == 0) return henyey_greenstein(wo, wi);
            else return {};
        }

        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> {
            if (idx == 0) return henyey_greenstein.sample(ctx, u);
            else return {};
        }

        public func empty() -> bool { return idx == 0xffffffffu; }
        [mutating] public func clear() -> void { idx = 0xffffffffu; }
    }
}
