#pragma once
#include <metatron/render/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>

namespace metatron::photo {
	struct Sensor {
		auto virtual to_rgb(spectra::Spectrum const& spectrum) -> math::Vector<f32, 3>;
	};
}
