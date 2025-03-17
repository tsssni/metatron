#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/spectra/stochastic.hpp>

namespace metatron::eval {
	struct Context final {
		math::Vector<f32, 3> const* p;
		math::Vector<f32, 3> const* n;
		math::Vector<f32, 2> const* uv;
		math::Ray const* r;
		spectra::Stochastic_Spectrum const* L;
	};
}
