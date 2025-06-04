#pragma once
#include <metatron/resource/shape/shape.hpp>

namespace metatron::shape {
	struct Plane final: Shape {
		Plane(f32 a, f32 b, f32 c, f32 d);
		Plane(math::Vector<f32, 3> const& p, math::Vector<f32, 3> const& n);
		auto bounding_box(usize idxuz) const -> math::Bounding_Box;
		auto operator()(
			math::Ray const& r,
			math::Vector<f32, 3> const& np = {},
			usize idx = 0uz
		) const -> std::optional<Interaction>;
		auto sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u,
			usize idxuz
		) const -> std::optional<Interaction>;
	
	private:
		f32 a;
		f32 b;
		f32 c;
		f32 d;
	};
}
