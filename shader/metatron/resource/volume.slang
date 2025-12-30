module volume;
__include volume.nanovdb;
__include volume.uniform;
import metatron.core;

namespace mtt::volume {
    public interface Volume {
        public func to_local(iv3 ijk) -> fv3;
        public func to_index(fv3 pos) -> iv3;
        public func dimensions() -> uv3;

        public func inside(iv3 pos) -> bool;
        public func inside(fv3 pos) -> bool;

        public func bounding_box() -> math::Bounding_Box;
        public func bounding_box(fv3 pos) -> math::Bounding_Box;
        public func bounding_box(iv3 ijk) -> math::Bounding_Box;

        public f32 operator()(fv3 pos);
        public __subscript(iv3 ijk) -> f32;
    }

    public struct Volume_Tag: Volume {
        tag<byte> idx;

        public func to_local(iv3 ijk) -> fv3 {
            if (idx.type() == 0) return tag<Uniform_Volume>(idx).get().to_local(ijk);
            else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get().to_local(ijk);
            else return {};
        }

        public func to_index(fv3 pos) -> iv3 {
            if (idx.type() == 0) return tag<Uniform_Volume>(idx).get().to_index(pos);
            else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get().to_index(pos);
            else return {};
        }

        public func dimensions() -> uv3 {
            if (idx.type() == 0) return tag<Uniform_Volume>(idx).get().dimensions();
            else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get().dimensions();
            else return {};
        }

        public func inside(iv3 pos) -> bool {
            if (idx.type() == 0) return tag<Uniform_Volume>(idx).get().inside(pos);
            else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get().inside(pos);
            else return {};
        }

        public func inside(fv3 pos) -> bool {
            if (idx.type() == 0) return tag<Uniform_Volume>(idx).get().inside(pos);
            else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get().inside(pos);
            else return {};
        }

        public func bounding_box() -> math::Bounding_Box {
            if (idx.type() == 0) return tag<Uniform_Volume>(idx).get().bounding_box();
            else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get().bounding_box();
            else return {};
        }

        public func bounding_box(fv3 pos) -> math::Bounding_Box {
            if (idx.type() == 0) return tag<Uniform_Volume>(idx).get().bounding_box(pos);
            else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get().bounding_box(pos);
            else return {};
        }

        public func bounding_box(iv3 ijk) -> math::Bounding_Box {
            if (idx.type() == 0) return tag<Uniform_Volume>(idx).get().bounding_box(ijk);
            else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get().bounding_box(ijk);
            else return {};
        }

        public f32 operator()(fv3 pos) {
            if (idx.type() == 0) return tag<Uniform_Volume>(idx).get()(pos);
            else if (idx.type() == 1) return tag<Nanovdb_Volume>(idx).get()(pos);
            else return {};
        }

        public __subscript(iv3 ijk) -> f32 {
            get {
                if (idx.type() == 0) return (*tag<Uniform_Volume>(idx).get())[ijk];
                else if (idx.type() == 1) return (*tag<Nanovdb_Volume>(idx).get())[ijk];
                else return {};
            }
        }
    }
}
