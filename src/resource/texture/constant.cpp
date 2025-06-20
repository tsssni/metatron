#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>

namespace metatron::texture {
	Constant_Texture<spectra::Stochastic_Spectrum>::Constant_Texture(
		std::unique_ptr<spectra::Spectrum> x
	): x(std::move(x)) {}

	auto Constant_Texture<spectra::Stochastic_Spectrum>::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) const -> spectra::Stochastic_Spectrum {
		return ctx.spec & (*x);
	}

	Constant_Texture<math::Vector<f32, 4>>::Constant_Texture(
		math::Vector<f32, 4> const& x
	): x(x) {}

	auto Constant_Texture<math::Vector<f32, 4>>::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) const -> math::Vector<f32, 4> {
		return x;
	}
}
