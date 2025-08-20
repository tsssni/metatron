#pragma once
#include <metatron/resource/texture/texture.hpp>

namespace mtt::texture {
	template<typename T>
	struct Constant_Texture final {};

	template<>
	struct Constant_Texture<spectra::Stochastic_Spectrum> final {
		Constant_Texture(view<spectra::Spectrum> x) noexcept;

		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const noexcept -> spectra::Stochastic_Spectrum;

	private:
		view<spectra::Spectrum> x;
	};

	template<>
	struct Constant_Texture<math::Vector<f32, 4>> final {
		Constant_Texture(math::Vector<f32, 4> const& x) noexcept;
		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const noexcept -> math::Vector<f32, 4>;

	private:
		math::Vector<f32, 4> x;
	};
}
