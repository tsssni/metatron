implementing volume;

namespace mtt::volume {
    public struct Nanovdb_Volume: Volume {
        public func to_local(iv3 ijk) -> fv3 {
            return {};
        }

        public func to_index(fv3 pos) -> iv3 {
            return {};
        }

        public func dimensions() -> uv3 {
            return {};
        }

        public func inside(iv3 pos) -> bool {
            return {};
        }

        public func inside(fv3 pos) -> bool {
            return {};
        }

        public func bounding_box() -> math::Bounding_Box {
            return {};
        }

        public func bounding_box(fv3 pos) -> math::Bounding_Box {
            return {};
        }

        public func bounding_box(iv3 ijk) -> math::Bounding_Box {
            return {};
        }

        public f32 operator()(fv3 pos) {
            return {};
        }

        public __subscript(iv3 ijk) -> f32 {
            get {
                return {};
            }
        }
    }
}
