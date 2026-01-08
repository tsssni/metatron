implementing volume;
#define PNANOVDB_HLSL
#define PNANOVDB_ADDRESS_64
#include "PNanoVDB.slang"

namespace mtt::volume {
    public struct Nanovdb_Volume: Volume {
        public tag<byte> handle;
        math::Bounding_Box bbox;

        func to(iv3 ijk) -> pnanovdb_coord_t { return {ijk.x, ijk.y, -ijk.z}; }
        func to(fv3 pos) -> pnanovdb_vec3_t { return {pos.x, pos.y, -pos.z}; }
        func from(pnanovdb_coord_t coord) -> iv3 { return {coord.x, coord.y, -coord.z}; }
        func from(pnanovdb_vec3_t vec) -> fv3 { return {vec.x, vec.y, -vec.z}; }

        func buf() -> Ptr<uint> {
            let arr = Ptr<uptr>(resources.volumes);
            return Ptr<uint>(arr[handle.index()]);
        }

        func grid() -> pnanovdb_grid_handle_t {
            var grid: pnanovdb_grid_handle_t;
            grid.address = pnanovdb_address_null();
            return grid;
        }

        func type() -> pnanovdb_grid_type_t {
            return pnanovdb_grid_get_grid_type(buf(), grid());
        }

        func tree() -> pnanovdb_tree_handle_t {
            return pnanovdb_grid_get_tree(buf(), grid());
        }

        func root() -> pnanovdb_root_handle_t {
            return pnanovdb_tree_get_root(buf(), tree());
        }

        func accessor() -> pnanovdb_readaccessor_t {
            var acc: pnanovdb_readaccessor_t;
            pnanovdb_readaccessor_init(acc, root());
            return acc;
        }

        public func to_local(iv3 ijk) -> fv3 {
            let pos = pnanovdb_grid_index_to_worldf(buf(), grid(), to(ijk));
            return from(pos);
        }
        public func to_index(fv3 pos) -> iv3 {
            let idx = pnanovdb_grid_world_to_indexf(buf(), grid(), to(pos));
            return from(iv3(floor(idx)));
        }
        public func dimensions() -> uv3 {
            var bbox_min = pnanovdb_root_get_bbox_min(buf(), root());
            var bbox_max = pnanovdb_root_get_bbox_max(buf(), root());
            return abs(bbox_max - bbox_min) + 1;
        }

        public func inside(iv3 pos) -> bool {
            var bbox_min = pnanovdb_root_get_bbox_min(buf(), root());
            var bbox_max = pnanovdb_root_get_bbox_max(buf(), root());
            return all(to(pos) >= bbox_min) && all(to(pos) <= bbox_max);
        }
        public func inside(fv3 pos) -> bool { return math::inside(pos, bbox); }

        public func bounding_box() -> math::Bounding_Box {
            return bbox;
        }
        public func bounding_box(fv3 pos) -> math::Bounding_Box {
            return bounding_box(to_index(pos));
        }
        public func bounding_box(iv3 ijk) -> math::Bounding_Box {
            if (inside(ijk)) return {to_local(ijk), to_local(ijk + 1)};
            else return bounding_box();
        }

        public f32 operator()(fv3 pos) {
            var acc = accessor();
            var bbox_min = pnanovdb_root_get_bbox_min(buf(), root());
            var bbox_max = pnanovdb_root_get_bbox_max(buf(), root());
            let idx = pnanovdb_grid_world_to_indexf(buf(), grid(), to(pos));

            var value = 0.f;
            let cell_min = iv3(floor(idx));
            let cell_max = cell_min + 1;
            let t = idx - cell_min;
            for (var i = 0; i < 8; ++i) {
                let choice = bool3(iv3(i & 1, (i >> 1) & 1, (i >> 2) & 1));
                let ijk = select(choice, cell_max, cell_min);
                if (any(ijk < bbox_min) || any(ijk > bbox_max)) continue;

                let weight = math::prod(select(choice, t, 1 - t));
                let address = pnanovdb_readaccessor_get_value_address(type(), buf(), acc, ijk);
                value += weight * pnanovdb_read_float(buf(), address);
            }
            return value;
        }

        public __subscript(iv3 ijk) -> f32 {get {
            let type = pnanovdb_grid_get_grid_type(buf(), grid());
            var acc = accessor();
            let address = pnanovdb_readaccessor_get_value_address(type, buf(), acc, to(ijk));
            return pnanovdb_read_float(buf(), address);
        }}
    }
}
