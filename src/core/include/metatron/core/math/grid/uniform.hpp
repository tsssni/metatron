#pragma once
#include <metatron/core/math/grid/grid.hpp>
#include <vector>

namespace mtt::math {
    template<typename T>
    struct Uniform_Grid final {
        Uniform_Grid(
            Bounding_Box const& bbox,
            Vector<usize, 3> const& dimensions
        ):
        bbox(bbox),
        dims(dimensions),
        voxel_size((bbox.p_max - bbox.p_min) / dimensions),
        data(math::prod(dimensions))
        {}

        auto to_local(Vector<i32, 3> const& ijk) const noexcept -> Vector<f32, 3> {
            return bbox.p_min + ijk * voxel_size;
        };

        auto to_index(Vector<f32, 3> const& pos) const noexcept -> Vector<i32, 3> {
            return floor((pos - bbox.p_min) / voxel_size);
        };

        auto dimensions() const noexcept -> math::Vector<usize, 3> {
            return dims;
        }

        auto inside(Vector<i32, 3> const& pos) const noexcept -> bool {
            return all(
                [](i32 p, i32 q, auto){return p >= 0 && p < q;},
                pos, dims
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
            if (ijk == clamp(ijk, Vector<i32, 3>{0}, dims - 1)) {
                return Bounding_Box{
                    bbox.p_min + Vector<f32, 3>(ijk + 0) * voxel_size,
                    bbox.p_min + Vector<f32, 3>(ijk + 1) * voxel_size,
                };
            } else {
                return bbox;
            }
        }

        auto operator()(Vector<f32, 3> const& pos) const noexcept -> T {
            return (*this)[to_index(pos)];
        }

        auto operator[](Vector<i32, 3> const& ijk) -> T& {
            auto [i, j, k] = ijk;
            auto offset = i * dims[1] * dims[2] + j * dims[2] + k;
            return data[offset];
        }

        auto operator[](Vector<i32, 3> const& ijk) const noexcept -> T const& {
            return const_cast<Uniform_Grid&>(*this)[ijk];
        }

    private:
        Bounding_Box bbox;
        Vector<i32, 3> dims;
        Vector<f32, 3> voxel_size;
        std::vector<T> data;
    };
}
