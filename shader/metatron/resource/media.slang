module media;
__include media.heterogeneous;
__include media.homogeneous;
__include media.vaccum;
import phase;
import spectra;
import volume;
import metatron.core;

#define ITER_V(method, ...) \
         if (idx == 0) return homogeneous.method(__VA_ARGS__); \
    else if (idx == 1) return heterogeneous.method(__VA_ARGS__); \
    else if (idx == 2) return vaccum.method(__VA_ARGS__);

#define ITER_R(method, ...) ITER_V(method, __VA_ARGS__) else return {};

#define MED_V(method, ...) \
         if (idx.type() == 0) return Iterator(tag<Homogeneous_Medium>(idx).get().method(__VA_ARGS__)); \
    else if (idx.type() == 1) return Iterator(tag<Heterogeneous_Medium>(idx).get().method(__VA_ARGS__)); \
    else if (idx.type() == 2) return Iterator(tag<Vaccum_Medium>(idx).get().method(__VA_ARGS__));

#define MED_R(method, ...) MED_V(method, __VA_ARGS__) else return {};

namespace mtt::media {
    public struct Interaction {
        public phase::Phase_Function phase;
        public fv3 p;
        public f32 t;
        public fv4 transmittance;
        public fv4 sigma_a;
        public fv4 sigma_s;
        public fv4 sigma_n;
        public fv4 sigma_maj;
        public fv4 sigma_e;
    }

    public struct Phase {
        enum Function {
            henyey_greenstein,
        } function = Function::henyey_greenstein;
        f32 g = 0.f;

        public func to_phase() -> phase::Phase_Function {
            switch (function) {
            case Function::henyey_greenstein: return {phase::Henyey_Greenstein_Phase_Function(g)};
            default: return {};
            }
        }
    }

    public struct Iterator {
        Homogeneous_Medium.Iterator homogeneous = {};
        Heterogeneous_Medium.Iterator heterogeneous = {};
        Vaccum_Medium.Iterator vaccum = {};
        u32 idx = 0xffffffffu;

        public __init() {}
        public __init(Homogeneous_Medium.Iterator it) { homogeneous = it; idx = 0; }
        public __init(Heterogeneous_Medium.Iterator it) { heterogeneous = it; idx = 1; }
        public __init(Vaccum_Medium.Iterator it) { vaccum = it; idx = 2; }

        [mutating] public func march(f32 u) -> Optional<Interaction> { ITER_R(march, u) }
    }

    public struct Medium {
        public tag<byte> idx;
        public func begin(math::Context ctx, f32 t_max) -> Iterator { MED_R(begin, ctx, t_max) }
    }
}

#undef ITER_V
#undef ITER_R
#undef MED_V
#undef MED_R
