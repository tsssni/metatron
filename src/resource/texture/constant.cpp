#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>

namespace mtt::texture {
	auto Constant_Texture<spectra::Stochastic_Spectrum>::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) const noexcept -> spectra::Stochastic_Spectrum {
		return ctx.spec & x;
	}

	Constant_Texture<math::Vector<f32, 4>>::Constant_Texture(
		math::Vector<f32, 4> const& x
	) noexcept: x(x) {}

	auto Constant_Texture<math::Vector<f32, 4>>::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) const noexcept -> math::Vector<f32, 4> {
		return x;
	}
}
