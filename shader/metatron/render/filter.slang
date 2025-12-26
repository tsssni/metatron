module filter;
__include filter.box;
__include filter.gaussian;
__include filter.lanczos;
import metatron.core;

namespace mtt::filter {
    public struct Interaction {
        public fv2 p;
        public f32 weight;
        public f32 pdf;
    }

    public interface Filter {
        public f32 operator()(fv2 p);
        public func sample(fv2 u) -> Optional<Interaction>;
    }

    public struct Filter_Union: Filter {
        Box_Filter box;
        Gaussian_Filter gaussian;
        Lanczos_Filter lanczos;
        u32 idx;

        public f32 operator()(fv2 p) {
            if (idx == 0) return box(p);
            else if (idx == 1) return gaussian(p);
            else if (idx == 2) return lanczos(p);
            else return {};
        }

        public func sample(fv2 u) -> Optional<Interaction> {
            if (idx == 0) return box.sample(u);
            else if (idx == 1) return gaussian.sample(u);
            else if (idx == 2) return lanczos.sample(u);
            else return {};
        }
    }
}
