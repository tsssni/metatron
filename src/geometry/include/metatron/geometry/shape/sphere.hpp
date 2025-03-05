#pragma once
#include <metatron/geometry/shape/shape.hpp>

namespace metatron::shape {
	struct Sphere final: Shape {
		Sphere(f32 radius, f32 theta_min, f32 theta_max, f32 phi_max);

		auto bounding_box(usize idx = 0uz) const -> intr::Bounding_Box;

		auto sample(
			intr::Context const& ctx,
			math::Vector<f32, 2> const& u,
			usize idx = 0uz
		) const -> std::optional<intr::Interaction>;

		auto intersect(
			math::Ray const& ctx,
			usize idx = 0uz
		) const -> std::optional<intr::Interaction>;

	private:
		f32 radius;
		f32 theta_min;
		f32 theta_max;
		f32 phi_max;
	};
}
