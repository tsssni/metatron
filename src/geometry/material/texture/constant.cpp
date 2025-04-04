#include <metatron/geometry/material/texture/constant.hpp>
#include <metatron/core/spectra/rgb.hpp>

namespace metatron::material {
	Constant_Texture<spectra::Stochastic_Spectrum>::Constant_Texture(std::unique_ptr<spectra::Spectrum> x)
		: x(std::move(x)) {}

	auto Constant_Texture<spectra::Stochastic_Spectrum>::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) const -> spectra::Stochastic_Spectrum {
		return ctx.L & (*x);
	}
}
