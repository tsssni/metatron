#pragma once
#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/eval/context.hpp>

namespace metatron::material {
	struct Coordinate final {
		math::Vector<f32, 2> uv;
		f32 dudx;
		f32 dudy;
		f32 dvdx;
		f32 dvdy;
	};

	template<typename T>
	struct Texture {
		using Element = T;

		virtual ~Texture() {}
		auto virtual sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) -> Element = 0;
	};
}
