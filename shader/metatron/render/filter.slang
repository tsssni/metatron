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

    public struct Filter_Union {
        Box_Filter box;
        Gaussian_Filter gaussian;
        Lanczos_Filter lanczos;
        u32 idx;

        public func get() -> Filter {
            if (idx == 0) return box;
            else if (idx == 1) return gaussian;
            else if (idx == 2) return lanczos;
            else return {};
        }
    }
}
