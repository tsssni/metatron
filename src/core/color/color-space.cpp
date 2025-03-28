#include <metatron/core/color/color-space.hpp>

namespace metatron::color {
	Color_Space::Color_Space(
		math::Vector<f32, 2> const& r,
		math::Vector<f32, 2> const& g,
		math::Vector<f32, 2> const& b,
		spectra::Spectrum const& white_point
	): white_point(&white_point) {

	}
}
