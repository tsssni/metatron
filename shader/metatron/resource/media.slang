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
        } function;
        f32 g;

        public func to_phase() -> phase::Phase_Function_Union {
            switch (function) {
            case Function::henyey_greenstein: return {phase::Henyey_Greenstein_Phase_Function(g)};
            default: return {};
            }
        }
    }

    public interface Medium {
        public func sample(math::Context ctx, f32 t_max, f32 u) -> Optional<Interaction>;
    }

    public struct Medium_Tag: Medium {
        public tag<byte> idx;

        public func sample(math::Context ctx, f32 t_max, f32 u) -> Optional<Interaction> {
            if (idx.type() == 0) return tag<Homogeneous_Medium>(idx).get().sample(ctx, t_max, u);
            else if (idx.type() == 1) return tag<Heterogeneous_Medium>(idx).get().sample(ctx, t_max, u);
            else if (idx.type() == 2) return tag<Vaccum_Medium>(idx).get().sample(ctx, t_max, u);
            else return {};
        }
    }
}
