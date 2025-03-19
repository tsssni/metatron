#include <metatron/geometry/material/texture/spectrum/constant.hpp>
#include <metatron/core/spectra/rgb.hpp>

namespace metatron::material {
	Spectrum_Constant_Texture::Spectrum_Constant_Texture(std::unique_ptr<spectra::Spectrum> x)
		: x(std::move(x)) {}

	auto Spectrum_Constant_Texture::sample(eval::Context const& ctx) -> Element {
		return ctx.L & (*x);
	}
}
