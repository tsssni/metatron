#pragma once
#include <metatron/resource/shape/sphere.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>

namespace metatron::texture {
	struct Coordinate final {
		math::Vector<f32, 2> uv{};
		f32 dudx{0.f};
		f32 dudy{0.f};
		f32 dvdx{0.f};
		f32 dvdy{0.f};
	};

	template<typename T>
	struct Texture {
		virtual ~Texture() {}
		auto virtual sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> T = 0;
	};
}
