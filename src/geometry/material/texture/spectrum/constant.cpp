#include <metatron/core/spectra/rgb.hpp>
#include <metatron/geometry/material/texture/spectrum/constant.hpp>

namespace metatron::material {
	Spectrum_Constant_Texture::Spectrum_Constant_Texture(std::unique_ptr<spectra::Spectrum> x)
		: x(std::move(x)) {}

	auto Spectrum_Constant_Texture::sample(Context const& ctx) -> Element {
		return std::make_unique<spectra::Stochastic_Spectrum>((*ctx.Lo) & (*x));
	}
}
