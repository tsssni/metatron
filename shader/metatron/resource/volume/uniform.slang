implementing volume;

namespace mtt::volume {
    public struct Uniform_Volume: Volume {
        math::Bounding_Box bbox;
        iv3 dims;
        fv3 voxel_size;
        tag<u32> idx;

        public func to_local(iv3 ijk) -> fv3 { return bbox.p_min + ijk * voxel_size; }
        public func to_index(fv3 pos) -> iv3 { return iv3(floor((pos - bbox.p_min) / voxel_size)); }
        public func dimensions() -> uv3 { return dims; }

        public func inside(iv3 pos) -> bool { return all(pos >= 0 && pos <= dims); }
        public func inside(fv3 pos) -> bool { return inside(to_index(pos)); }

        public func bounding_box() -> math::Bounding_Box { return bbox; }
        public func bounding_box(fv3 pos) -> math::Bounding_Box { return bounding_box(to_index(pos)); }
        public func bounding_box(iv3 ijk) -> math::Bounding_Box {
            if (all(ijk == clamp(ijk, iv3(0), dims - 1))) return {
                bbox.p_min + fv3(ijk + 0) * voxel_size,
                bbox.p_min + fv3(ijk + 1) * voxel_size,
            }; else return bbox;
        }

        public f32 operator()(fv3 pos) { return this[to_index(pos)]; }
        public __subscript(iv3 ijk) -> f32 {
            get {
                let grid = global.grids[idx.index()];
                return grid.Load(iv4(ijk, 0)).x;
            }
        }
    }
}
