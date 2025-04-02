#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/spectra/stochastic.hpp>

namespace metatron::eval {
	struct Context final {
		math::Vector<f32, 3> p{};
		math::Vector<f32, 3> n{};
		math::Ray r{};
		spectra::Stochastic_Spectrum L{};
	};
}
