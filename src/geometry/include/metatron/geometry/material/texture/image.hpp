#pragma once
#include <metatron/core/image/image.hpp>
#include <metatron/core/color/color-space.hpp>
#include <metatron/geometry/material/texture/texture.hpp>

namespace metatron::material {
	template<typename T>
	struct Image_Texture final: Texture<T> {};

	template<>
	struct Image_Texture<spectra::Stochastic_Spectrum> final: Texture<spectra::Stochastic_Spectrum> {
		Image_Texture(
			std::unique_ptr<image::Image> image,
			color::Color_Space::Spectrum_Type spectrum_type,
			usize mips = 1
		);

		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> spectra::Stochastic_Spectrum;

	private:
		std::vector<std::unique_ptr<image::Image>> images;
		color::Color_Space::Spectrum_Type spectrum_type;
	};
}
