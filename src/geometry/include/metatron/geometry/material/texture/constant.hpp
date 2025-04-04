#pragma once
#include <metatron/core/image/image.hpp>
#include <metatron/geometry/material/texture/texture.hpp>

namespace metatron::material {
	template<typename T>
	struct Constant_Texture final: Texture<T> {};

	template<>
	struct Constant_Texture<spectra::Stochastic_Spectrum> final: Texture<spectra::Stochastic_Spectrum> {

		Constant_Texture(std::unique_ptr<spectra::Spectrum> x);
		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> spectra::Stochastic_Spectrum;

	private:
		std::unique_ptr<spectra::Spectrum> x;
	};
}
