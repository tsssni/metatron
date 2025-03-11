#pragma once
#include <metatron/core/math/ray.hpp>
#include <metatron/core/spectra/spectrum.hpp>

namespace metatron::media {
	struct Context final {
		math::Ray r;
		f32 lambda;
	};

	struct Interaction final {
		math::Vector<f32, 3> p;
		spectra::Spectrum const* sigma_a;
		spectra::Spectrum const* sigma_s;
	};

	struct Medium {
		auto virtual sample(Context const& ctx, f32 u) -> Interaction = 0;
	};
}
