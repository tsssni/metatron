#pragma once
#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/core/eval/context.hpp>

namespace metatron::material {
	template<typename T>
	struct Texture {
		using Element = T;
		virtual ~Texture() {}
		auto virtual sample(eval::Context const& ctx) -> Element = 0;
	};
}
