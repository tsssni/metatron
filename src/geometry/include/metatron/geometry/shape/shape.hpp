#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <optional>

namespace metatron::shape {
	struct Interaction final {
		math::Vector<f32, 3> p;
		math::Vector<f32, 3> n;
		math::Vector<f32, 2> uv;
		f32 pdf;
	};

	struct Context final {
		math::Vector<f32, 3> p;
		math::Vector<f32, 3> n;
		math::Ray r;
	};

	struct Shape {
		virtual ~Shape() {}
		auto virtual bounding_box(usize idx = 0uz) const -> math::Bounding_Box = 0;
		auto virtual sample(
			Context const& ctx,
			math::Vector<f32, 2> const& u,
			usize idx = 0uz
		) const -> std::optional<Interaction> = 0;
		auto virtual intersect(
			math::Ray const& r,
			usize idx = 0uz
		) const -> std::optional<Interaction> = 0;
	};
}
