module shape;
__include shape.sphere;
import metatron.core;

namespace mtt::shape {
    public struct Interaction {
        public fv3 p;
        public fv3 n;
        public fv3 tn;
        public fv3 bn;
        public fv2 uv;
        public f32 t;
        public f32 pdf;

        public fv3 dpdu;
        public fv3 dpdv;
        public fv3 dndu;
        public fv3 dndv;
    }

    public interface Shape {
        public func size() -> u32;
        public func bounding_box(math::Transform t, u32 idx) -> math::Bounding_Box;
        public func intersect(math::Ray r, fv3 np, u32 idx) -> Optional<Interaction>;
        public func sample(math::Context ctx, fv2 u, u32 idx) -> Optional<Interaction>;
        public func query(math::Ray r, u32 idx) -> Optional<f32>;
    }

    public struct Shape_Tag {
        tag<byte> idx;

        public func get() -> Shape {
            if (idx.type() == 0) return {};
            else if (idx.type() == 1) return tag<Sphere>(idx).get();
            else return {};
        }
    }
}
