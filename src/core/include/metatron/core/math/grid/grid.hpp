#pragma once
#include <metatron/core/math/bounding-box.hpp>

namespace metatron::math {
	template<typename T, usize x, usize y, usize z>
	struct Grid {
		auto static constexpr dimensions = std::array<usize, 3>{x, y, z};
		virtual ~Grid() = default;

		auto virtual to_local(math::Vector<usize, 3> const& ijk) const -> math::Vector<f32, 3> = 0;
		auto virtual to_index(math::Vector<f32, 3> const& pos) const -> math::Vector<usize, 3> = 0;

		auto virtual bounding_box() const -> math::Bounding_Box = 0;
		auto virtual bounding_box(math::Vector<f32, 3> const& pos) const -> math::Bounding_Box = 0;
		auto virtual bounding_box(math::Vector<usize, 3> const& ijk) const -> math::Bounding_Box = 0;

		auto virtual operator()(math::Vector<f32, 3> const& pos) -> T& = 0;
		auto virtual operator()(math::Vector<f32, 3> const& pos) const -> T const& = 0;
		auto virtual operator[](math::Vector<usize, 3> const& ijk) -> T& = 0;
		auto virtual operator[](math::Vector<usize, 3> const& ijk) const -> T const& = 0;
	};
}
