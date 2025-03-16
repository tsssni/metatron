#pragma once
#include <metatron/geometry/shape/sphere.hpp>
#include <metatron/core/spectra/stochastic.hpp>

namespace metatron::material {
	struct Context final {
		shape::Interaction const* intr;
		spectra::Stochastic_Spectrum const* Lo{nullptr};
	};

	template<typename T>
	struct Texture {
		using Element = T;
		virtual ~Texture() {}
		auto virtual sample(Context const& ctx) -> Element = 0;
	};
}
