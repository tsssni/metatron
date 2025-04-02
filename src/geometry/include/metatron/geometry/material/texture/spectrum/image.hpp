#pragma once
#include <metatron/core/image/image.hpp>
#include <metatron/core/color/color-space.hpp>
#include <metatron/geometry/material/texture/spectrum/spectrum.hpp>

namespace metatron::material {
	struct Spectrum_Image_Texture final: Spectrum_Texture {
		Spectrum_Image_Texture(std::unique_ptr<image::Image> image, color::Color_Space::Spectrum_Type spectrum_type);
		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) -> Element;

	private:
		std::unique_ptr<image::Image> image;
		color::Color_Space::Spectrum_Type spectrum_type;
	};
}
