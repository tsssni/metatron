implementing shape;

namespace mtt::shape {
    public struct Sphere: Shape {
        public func size() -> u32 { return 1; }

        public func bounding_box(math::Transform t, u32 idx = 0) -> math::Bounding_Box {
            let bbox = math::Bounding_Box{{-1, -1, -1}, {1, 1, 1}};
            return t * bbox;
        }

        public func intersect(math::Ray r, fv3 np, u32 idx) -> Optional<Interaction> {
            return none;
        }

        public func sample(math::Context ctx, fv2 u, u32 idx) -> Optional<Interaction> {
            return none;
        }

        public func query(math::Ray r, u32 idx) -> Optional<f32> {
            return none;
        }
    }
}
