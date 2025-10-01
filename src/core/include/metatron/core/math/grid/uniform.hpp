#pragma once
#include <metatron/core/math/grid/grid.hpp>

namespace mtt::math {
    template<typename T, usize x, usize y, usize z>
    struct Uniform_Grid final {
        auto static constexpr dimensions = math::Vector<usize, 3>{x, y, z};

        Uniform_Grid(Bounding_Box const& bbox):
        bbox(bbox), 
        voxel_size((bbox.p_max - bbox.p_min) / math::Vector<f32, 3>{x, y, z})
        {}

        auto to_local(math::Vector<i32, 3> const& ijk) const noexcept -> math::Vector<f32, 3> {
            return bbox.p_min + ijk * voxel_size;
        };

        auto to_index(math::Vector<f32, 3> const& pos) const noexcept -> math::Vector<i32, 3> {
            return (pos - bbox.p_min) / voxel_size;
        };

        auto bounding_box() const noexcept -> Bounding_Box {
            return bbox;
        }

        auto bounding_box(Vector<f32, 3> const& pos) const noexcept -> Bounding_Box {
            return bounding_box(to_index(pos));
        }

        auto bounding_box(Vector<i32, 3> const& ijk) const noexcept -> Bounding_Box {
            if (ijk == clamp(ijk, Vector<i32, 3>{0}, Vector<i32, 3>{x - 1, y - 1, z - 1})) {
                return Bounding_Box{
                    bbox.p_min + math::Vector<f32, 3>(ijk + 0) * voxel_size,
                    bbox.p_min + math::Vector<f32, 3>(ijk + 1) * voxel_size,
                };
            } else {
                return bbox;
            }
        }

        auto operator()(math::Vector<f32, 3> const& pos) const noexcept -> T {
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
        math::Vector<f32, 3> voxel_size;
        Matrix<T, x, y, z> data;
    };
}
