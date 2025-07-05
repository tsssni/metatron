#pragma once
#include <metatron/resource/shape/shape.hpp>

namespace mtt::shape {
	struct Sphere final: Shape {
		auto size() const -> usize;
		auto bounding_box(
			math::Matrix<f32, 4, 4> const& t,
			usize idx
		) const -> math::Bounding_Box;
		auto operator()(
			math::Ray const& r,
			math::Vector<f32, 3> const& np = {},
			usize idx = 0uz
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u,
			usize idx = 0uz
		) const -> std::optional<Interaction>;
	};
}
