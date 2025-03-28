#pragma once
#include <metatron/core/math/vector.hpp>
#include <metatron/core/spectra/spectrum.hpp>

namespace metatron::color {
	struct Color_Space final {
		math::Matrix<f32, 3, 3> from_XYZ;
		math::Matrix<f32, 3, 3> to_XYZ;
		spectra::Spectrum const* white_point;

		Color_Space(
			math::Vector<f32, 2> const& r,
			math::Vector<f32, 2> const& g,
			math::Vector<f32, 2> const& b,
			spectra::Spectrum const& white_point
		);
	};
}
