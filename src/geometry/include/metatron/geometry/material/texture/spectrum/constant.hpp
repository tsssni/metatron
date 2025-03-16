#pragma once
#include <metatron/core/image/image.hpp>
#include <metatron/geometry/material/texture/spectrum/spectrum.hpp>

namespace metatron::material {
	struct Spectrum_Constant_Texture final: Spectrum_Texture {
		Spectrum_Constant_Texture(std::unique_ptr<spectra::Spectrum> x);
		auto sample(Context const& ctx) -> Element;

	private:
		std::unique_ptr<spectra::Spectrum> x;
	};
}
