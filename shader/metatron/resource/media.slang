module media;
__include media.heterogeneous;
__include media.homogeneous;
__include media.vaccum;
import phase;
import spectra;
import volume;
import metatron.core;

namespace mtt::media {
    public struct Interaction {
        public phase::Phase_Function_Union phase;
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

        public func to_phase() -> phase::Phase_Function_Union {
            switch (function) {
            case Function::henyey_greenstein: return {phase::Henyey_Greenstein_Phase_Function(g)};
            default: return {};
            }
        }
    }

    public struct Iterator_Union {
        Homogeneous_Medium.Iterator homogeneous = {};
        Heterogeneous_Medium.Iterator heterogeneous = {};
        Vaccum_Medium.Iterator vaccum = {};
        u32 idx = 0xffffffffu;

        public __init() {}
        public __init(Homogeneous_Medium.Iterator it) { homogeneous = it; idx = 0; }
        public __init(Heterogeneous_Medium.Iterator it) { heterogeneous = it; idx = 1; }
        public __init(Vaccum_Medium.Iterator it) { vaccum = it; idx = 2; }

        [mutating] public func march(f32 u) -> Optional<Interaction> {
            if (idx == 0) return homogeneous.march(u);
            else if (idx == 1) return heterogeneous.march(u);
            else if (idx == 2) return vaccum.march(u);
            else return {};
        }
    }

    public struct Medium_Tag {
        public tag<byte> idx;

        public func begin(math::Context ctx, f32 t_max) -> Iterator_Union {
            if (idx.type() == 0) return Iterator_Union(tag<Homogeneous_Medium>(idx).get().begin(ctx, t_max));
            else if (idx.type() == 1) return Iterator_Union(tag<Heterogeneous_Medium>(idx).get().begin(ctx, t_max));
            else if (idx.type() == 2) return Iterator_Union(tag<Vaccum_Medium>(idx).get().begin(ctx, t_max));
            else return {};
        }
    }
}
