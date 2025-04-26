#pragma once
#include <metatron/core/math/grid/grid.hpp>

namespace metatron::math {
	template<typename T, usize x, usize y, usize z>
	struct Uniform_Grid final: Grid<T, x, y, z> {
		Uniform_Grid(Bounding_Box const& bbox):
		bbox(bbox), 
		voxel_size((bbox.p_max - bbox.p_min) / math::Vector<f32, 3>{x, y, z})
		{}

		auto virtual to_local(math::Vector<i32, 3> const& ijk) const -> math::Vector<f32, 3> {
			return bbox.p_min + ijk * voxel_size;
		};

		auto virtual to_index(math::Vector<f32, 3> const& pos) const -> math::Vector<i32, 3> {
			return (pos - bbox.p_min) / voxel_size;
		};

		auto virtual bounding_box() const -> Bounding_Box {
			return bbox;
		}

		auto virtual bounding_box(Vector<f32, 3> const& pos) const -> Bounding_Box {
			return bounding_box(to_index(pos));
		}

		auto virtual bounding_box(Vector<i32, 3> const& ijk) const -> Bounding_Box {
			if (ijk == clamp(ijk, Vector<i32, 3>{0}, Vector<i32, 3>{x - 1, y - 1, z - 1})) {
				return Bounding_Box{
					bbox.p_min + math::Vector<f32, 3>(ijk + 0) * voxel_size,
					bbox.p_min + math::Vector<f32, 3>(ijk + 1) * voxel_size,
				};
			} else {
				return bbox;
			}
		}

		auto virtual operator()(math::Vector<f32, 3> const& pos) -> T& {
			return (*this)[to_index(pos)];
		}

		auto virtual operator()(math::Vector<f32, 3> const& pos) const -> T const& {
			return const_cast<Uniform_Grid&>(*this)(pos);
		}

		auto virtual operator[](Vector<i32, 3> const& ijk) -> T& {
			return data[ijk[0]][ijk[1]][ijk[2]];
		}

		auto virtual operator[](Vector<i32, 3> const& ijk) const -> T const& {
			return const_cast<Uniform_Grid&>(*this)[ijk];
		}

	private:
		Bounding_Box bbox;
		math::Vector<f32, 3> voxel_size;
		Matrix<T, x, y, z> data;
	};
}
