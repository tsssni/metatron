#pragma once
#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/eval/context.hpp>

namespace metatron::material {
	struct Coordinate final {
		math::Vector<f32, 2> uv{};
		f32 dudx{0.f};
		f32 dudy{0.f};
		f32 dvdx{0.f};
		f32 dvdy{0.f};
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
