#pragma once
#include <metatron/resource/texture/texture.hpp>

namespace mtt::texture {
	struct Constant_Spectrum_Texture final {
		Constant_Spectrum_Texture(view<spectra::Spectrum> x) noexcept;

		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const noexcept -> spectra::Stochastic_Spectrum;

	private:
		view<spectra::Spectrum> x;
	};

	struct Constant_Vector_Texture final {
		Constant_Vector_Texture(math::Vector<f32, 4> const& x) noexcept;
		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const noexcept -> math::Vector<f32, 4>;

	private:
		math::Vector<f32, 4> x;
	};
}
