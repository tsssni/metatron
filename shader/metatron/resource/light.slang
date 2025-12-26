module light;
__include light.area;
__include light.environment;
__include light.parallel;
__include light.point;
__include light.spot;
__include light.sunsky;

import spectra;
import shape;
import texture;
import metatron.core;

namespace mtt::light {
    public struct Interaction {
        public fv4 L;
        public fv3 wi;
        public fv3 p;
        public f32 t;
        public f32 pdf;
    }

    public enum Flags {
        delta = 1 << 0,
        inf = 1 << 1,
    }

    public interface Light {
        public Optional<Interaction> operator()(
            math::Ray r, fv4 lambda
        );
        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction>;
        public func flags() -> Flags;
    }

    public struct Light_Tag: Light {
        tag<byte> idx;
        public Optional<Interaction> operator()(
            math::Ray r, fv4 lambda
        ) {
            if (idx.type() == 0) return tag<Parallel_Light>(idx).get()(r, lambda);
            else if (idx.type() == 1) return tag<Point_Light>(idx).get()(r, lambda);
            else if (idx.type() == 2) return tag<Spot_Light>(idx).get()(r, lambda);
            else if (idx.type() == 3) return tag<Area_Light>(idx).get()(r, lambda);
            else if (idx.type() == 4) return tag<Environment_Light>(idx).get()(r, lambda);
            else if (idx.type() == 5) return tag<Sunsky_Light>(idx).get()(r, lambda);
            else return {};
        }

        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> {
            if (idx.type() == 0) return tag<Parallel_Light>(idx).get().sample(ctx, u);
            else if (idx.type() == 1) return tag<Point_Light>(idx).get().sample(ctx, u);
            else if (idx.type() == 2) return tag<Spot_Light>(idx).get().sample(ctx, u);
            else if (idx.type() == 3) return tag<Area_Light>(idx).get().sample(ctx, u);
            else if (idx.type() == 4) return tag<Environment_Light>(idx).get().sample(ctx, u);
            else if (idx.type() == 5) return tag<Sunsky_Light>(idx).get().sample(ctx, u);
            else return {};
        }

        public func flags() -> Flags {
            if (idx.type() == 0) return tag<Parallel_Light>(idx).get().flags();
            else if (idx.type() == 1) return tag<Point_Light>(idx).get().flags();
            else if (idx.type() == 2) return tag<Spot_Light>(idx).get().flags();
            else if (idx.type() == 3) return tag<Area_Light>(idx).get().flags();
            else if (idx.type() == 4) return tag<Environment_Light>(idx).get().flags();
            else if (idx.type() == 5) return tag<Sunsky_Light>(idx).get().flags();
            else return {};
        }
    }
}
