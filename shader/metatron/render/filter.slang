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

    public struct Filter_Tag: Filter {
        tag<byte> idx;

        public f32 operator()(fv2 p) {
            if (idx.type() == 0) return tag<Box_Filter>(idx).get()(p);
            else if (idx.type() == 1) return tag<Gaussian_Filter>(idx).get()(p);
            else if (idx.type() == 2) return tag<Lanczos_Filter>(idx).get()(p);
            else return {};
        }

        public func sample(fv2 u) -> Optional<Interaction> {
            if (idx.type() == 0) return tag<Box_Filter>(idx).get().sample(u);
            else if (idx.type() == 1) return tag<Gaussian_Filter>(idx).get().sample(u);
            else if (idx.type() == 2) return tag<Lanczos_Filter>(idx).get().sample(u);
            else return {};
        }
    }
}
