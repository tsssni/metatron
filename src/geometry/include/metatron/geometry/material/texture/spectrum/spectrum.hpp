#pragma once
#include <metatron/core/spectra/spectrum.hpp>
#include <metatron/geometry/material/texture/texture.hpp>
#include <memory>

namespace metatron::material {
	using Spectrum_Texture = Texture<std::unique_ptr<spectra::Spectrum>>;
}
