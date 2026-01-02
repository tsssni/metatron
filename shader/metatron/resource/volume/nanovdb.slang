implementing volume;
#define PNANOVDB_HLSL
#define PNANOVDB_ADDRESS_64
#include "PNanoVDB.slang"

namespace mtt::volume {
    func to_nanovdb_coord(iv3 ijk) -> pnanovdb_coord_t {
        return int3(ijk.x, ijk.y, -ijk.z);
    }

    func to_nanovdb_vec3(fv3 pos) -> pnanovdb_vec3_t {
        return float3(pos.x, pos.y, -pos.z);
    }

    func from_nanovdb_coord(pnanovdb_coord_t coord) -> iv3 {
        return iv3(coord.x, coord.y, -coord.z);
    }

    func from_nanovdb_vec3(pnanovdb_vec3_t vec) -> fv3 {
        return fv3(vec.x, vec.y, -vec.z);
    }

    public struct Nanovdb_Volume: Volume {
        tag<u32> handle;

        func buf() -> pnanovdb_buf_t {
            let arr = Ptr<uptr>(resources.volumes);
            return pnanovdb_buf_t(arr[handle.index()]);
        }

        func grid() -> pnanovdb_grid_handle_t {
            var grid: pnanovdb_grid_handle_t;
            grid.address = pnanovdb_address_null();
            return grid;
        }

        func root() -> pnanovdb_root_handle_t {
            var buf = this.buf();
            var grid = this.grid();
            var tree = pnanovdb_grid_get_tree(buf, grid);
            return pnanovdb_tree_get_root(buf, tree);
        }

        public func to_local(iv3 ijk) -> fv3 {
            var buf = this.buf();
            var grid = this.grid();
            var vdb_ijk = to_nanovdb_vec3(fv3(ijk));
            var world_pos = pnanovdb_grid_index_to_worldf(buf, grid, vdb_ijk);
            return from_nanovdb_vec3(world_pos);
        }

        public func to_index(fv3 pos) -> iv3 {
            var buf = this.buf();
            var grid = this.grid();
            var vdb_pos = to_nanovdb_vec3(pos);
            var index_pos = pnanovdb_grid_world_to_indexf(buf, grid, vdb_pos);
            return from_nanovdb_coord(iv3(floor(index_pos)));
        }

        public func dimensions() -> uv3 {
            var buf = this.buf();
            var root = this.root();
            var bbox_min = pnanovdb_root_get_bbox_min(buf, root);
            var bbox_max = pnanovdb_root_get_bbox_max(buf, root);
            var min_coord = from_nanovdb_coord(bbox_min);
            var max_coord = from_nanovdb_coord(bbox_max);
            return abs(max_coord - min_coord) + 1;
        }

        public func inside(iv3 pos) -> bool {
            var buf = this.buf();
            var root = this.root();
            var bbox_min = pnanovdb_root_get_bbox_min(buf, root);
            var bbox_max = pnanovdb_root_get_bbox_max(buf, root);
            var min_coord = from_nanovdb_coord(bbox_min);
            var max_coord = from_nanovdb_coord(bbox_max);
            var fpos = fv3(pos);
            return all(fpos >= fv3(min_coord)) && all(fpos <= fv3(max_coord));
        }

        public func inside(fv3 pos) -> bool {
            return inside(to_index(pos));
        }

        public func bounding_box() -> math::Bounding_Box {
            var buf = this.buf();
            var grid = this.grid();
            var min_x = f32(pnanovdb_grid_get_world_bbox(buf, grid, 0));
            var min_y = f32(pnanovdb_grid_get_world_bbox(buf, grid, 1));
            var min_z = f32(pnanovdb_grid_get_world_bbox(buf, grid, 2));
            var max_x = f32(pnanovdb_grid_get_world_bbox(buf, grid, 3));
            var max_y = f32(pnanovdb_grid_get_world_bbox(buf, grid, 4));
            var max_z = f32(pnanovdb_grid_get_world_bbox(buf, grid, 5));

            var p_min = from_nanovdb_vec3(fv3(min_x, min_y, min_z));
            var p_max = from_nanovdb_vec3(fv3(max_x, max_y, max_z));
            math::swap(p_min.z, p_max.z);
            return {p_min, p_max};
        }

        public func bounding_box(fv3 pos) -> math::Bounding_Box {
            return bounding_box(to_index(pos));
        }

        public func bounding_box(iv3 ijk) -> math::Bounding_Box {
            if (inside(ijk)) {
                return {to_local(ijk), to_local(ijk + 1)};
            } else {
                return bounding_box();
            }
        }

        public f32 operator()(fv3 pos) {
            var buf = this.buf();
            var grid = this.grid();
            var tree = pnanovdb_grid_get_tree(buf, grid);
            var root = pnanovdb_tree_get_root(buf, tree);
            var acc: pnanovdb_readaccessor_t;
            pnanovdb_readaccessor_init(acc, root);

            var vdb_pos = to_nanovdb_vec3(pos);
            var index_pos = pnanovdb_grid_world_to_indexf(buf, grid, vdb_pos);
            var ijk = iv3(floor(index_pos));
            var address = pnanovdb_readaccessor_get_value_address(PNANOVDB_GRID_TYPE_FLOAT, buf, acc, ijk);
            return pnanovdb_read_float(buf, address);
        }

        public __subscript(iv3 ijk) -> f32 {
            get {
                var buf = this.buf();
                var grid = this.grid();
                var tree = pnanovdb_grid_get_tree(buf, grid);
                var root = pnanovdb_tree_get_root(buf, tree);
                var acc: pnanovdb_readaccessor_t;
                pnanovdb_readaccessor_init(acc, root);

                var vdb_ijk = to_nanovdb_coord(ijk);
                var address = pnanovdb_readaccessor_get_value_address(PNANOVDB_GRID_TYPE_FLOAT, buf, acc, vdb_ijk);
                return pnanovdb_read_float(buf, address);
            }
        }
    }
}
