#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/spectra/stochastic.hpp>

namespace metatron::eval {
	struct Context final {
		math::Ray r{};
		math::Vector<f32, 3> n{};
		spectra::Stochastic_Spectrum L{};
	};
}
