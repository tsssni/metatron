#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/eval/context.hpp>
#include <optional>

namespace metatron::shape {
	struct Interaction final {
		math::Vector<f32, 3> p;
		math::Vector<f32, 3> n;
		math::Vector<f32, 2> uv;
		math::Vector<math::Vector<f32, 3>, 2> dp;
		math::Vector<math::Vector<f32, 3>, 2> dn;
		f32 t;
		f32 pdf;
	};

	struct Shape {
		virtual ~Shape() {}
		auto virtual bounding_box(usize idx = 0uz) const -> math::Bounding_Box = 0;
		auto virtual operator()(
			math::Ray const& r,
			usize idx = 0uz
		) const -> std::optional<Interaction> = 0;
		auto virtual operator()(
			math::Ray_Differential const& rd,
			usize idx = 0uz
		) const -> std::optional<Interaction> = 0;
		auto virtual sample(
			eval::Context const& ctx,
			math::Vector<f32, 2> const& u,
			usize idx = 0uz
		) const -> std::optional<Interaction> = 0;
	};
}
