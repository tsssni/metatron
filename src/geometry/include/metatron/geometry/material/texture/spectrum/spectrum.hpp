#pragma once
#include <metatron/core/spectra/spectrum.hpp>
#include <metatron/geometry/material/texture/texture.hpp>

namespace metatron::material {
	using Spectrum_Texture = Texture<spectra::Stochastic_Spectrum>;
}
