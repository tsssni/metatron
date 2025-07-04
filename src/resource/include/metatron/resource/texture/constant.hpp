#pragma once
#include <metatron/resource/image/image.hpp>
#include <metatron/resource/texture/texture.hpp>

namespace metatron::texture {
	template<typename T>
	struct Constant_Texture final: Texture<T> {};

	template<>
	struct Constant_Texture<spectra::Stochastic_Spectrum> final: Texture<spectra::Stochastic_Spectrum> {
		Constant_Texture(spectra::Spectrum const* x);
		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> spectra::Stochastic_Spectrum;

	private:
		spectra::Spectrum const* x;
	};

	template<>
	struct Constant_Texture<math::Vector<f32, 4>> final: Texture<math::Vector<f32, 4>> {
		Constant_Texture(math::Vector<f32, 4> const& x);
		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> math::Vector<f32, 4>;

	private:
		math::Vector<f32, 4> x;
	};
}
