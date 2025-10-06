#pragma once
#include <metatron/core/math/grid/grid.hpp>

namespace mtt::math {
    template<typename T, usize x, usize y, usize z>
    struct Uniform_Grid final {
        auto static constexpr dimensions = Vector<usize, 3>{x, y, z};

        Uniform_Grid(Bounding_Box const& bbox):
        bbox(bbox), 
        voxel_size((bbox.p_max - bbox.p_min) / Vector<f32, 3>{x, y, z})
        {}

        auto to_local(Vector<i32, 3> const& ijk) const noexcept -> Vector<f32, 3> {
            return bbox.p_min + ijk * voxel_size;
        };

        auto to_index(Vector<f32, 3> const& pos) const noexcept -> Vector<i32, 3> {
            return floor((pos - bbox.p_min) / voxel_size);
        };

        auto inside(Vector<i32, 3> const& pos) const noexcept -> bool {
            return all(
                [](i32 p, i32 q, auto){return p >= 0 && p < q;},
                pos, dimensions
            );
        }

        auto inside(Vector<f32, 3> const& pos) const noexcept -> bool {
            return inside(to_index(pos));
        }

        auto bounding_box() const noexcept -> Bounding_Box {
            return bbox;
        }

        auto bounding_box(Vector<f32, 3> const& pos) const noexcept -> Bounding_Box {
            return bounding_box(to_index(pos));
        }

        auto bounding_box(Vector<i32, 3> const& ijk) const noexcept -> Bounding_Box {
            if (ijk == clamp(ijk, Vector<i32, 3>{0}, Vector<i32, 3>{x - 1, y - 1, z - 1})) {
                return Bounding_Box{
                    bbox.p_min + Vector<f32, 3>(ijk + 0) * voxel_size,
                    bbox.p_min + Vector<f32, 3>(ijk + 1) * voxel_size,
                };
            } else {
                return bbox;
            }
        }

        auto operator()(Vector<f32, 3> const& pos) const noexcept -> T {
            return const_cast<Uniform_Grid&>(*this)(pos);
        }

        auto operator[](Vector<i32, 3> const& ijk) -> T& {
            return data[ijk[0]][ijk[1]][ijk[2]];
        }

        auto operator[](Vector<i32, 3> const& ijk) const noexcept -> T const& {
            return const_cast<Uniform_Grid&>(*this)[ijk];
        }

    private:
        Bounding_Box bbox;
        Vector<f32, 3> voxel_size;
        Matrix<T, x, y, z> data;
    };
}
