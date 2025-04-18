#pragma once
#include <metatron/geometry/shape/shape.hpp>

namespace metatron::shape {
	struct Sphere final: Shape {
		auto bounding_box(usize idx = 0uz) const -> math::Bounding_Box;
		auto operator()(
			math::Ray const& r,
			usize idx = 0uz
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u,
			usize idx = 0uz
		) const -> std::optional<Interaction>;
	};
}
