module shape;
__include shape.mesh;
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
        public func bounding_box(math::Transform t, u32 i) -> math::Bounding_Box;
        public Optional<Interaction> operator()(math::Ray r, fv3 np, fv4 pos, u32 i);
        public func sample(math::Context ctx, fv2 u, u32 i) -> Optional<Interaction>;
        public func query(math::Ray r, u32 i) -> Optional<fv4>;
    }

    public struct Shape_Tag: Shape {
        tag<byte> idx;

        public func size() -> u32 {
            if (idx.type() == 0) return tag<Mesh>(idx).get().size();
            else if (idx.type() == 1) return tag<Sphere>(idx).get().size();
            else return {};
        }

        public func bounding_box(math::Transform t, u32 i) -> math::Bounding_Box {
            if (idx.type() == 0) return tag<Mesh>(idx).get().bounding_box(t, i);
            else if (idx.type() == 1) return tag<Sphere>(idx).get().bounding_box(t, i);
            else return {};
        }

        public Optional<Interaction> operator()(math::Ray r, fv3 np, fv4 pos, u32 i) {
            if (idx.type() == 0) return tag<Mesh>(idx).get()(r, np, pos, i);
            else if (idx.type() == 1) return tag<Sphere>(idx).get()(r, np, pos, i);
            else return {};
        }

        public func sample(math::Context ctx, fv2 u, u32 i) -> Optional<Interaction> {
            if (idx.type() == 0) return tag<Mesh>(idx).get().sample(ctx, u, i);
            else if (idx.type() == 1) return tag<Sphere>(idx).get().sample(ctx, u, i);
            else return {};
        }

        public func query(math::Ray r, u32 i) -> Optional<fv4> {
            if (idx.type() == 0) return tag<Mesh>(idx).get().query(r, i);
            else if (idx.type() == 1) return tag<Sphere>(idx).get().query(r, i);
            else return {};
        }
    }
}
