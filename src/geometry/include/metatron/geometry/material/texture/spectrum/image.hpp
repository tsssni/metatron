#pragma once
#include <metatron/core/image/image.hpp>
#include <metatron/geometry/material/texture/spectrum/spectrum.hpp>

namespace metatron::material {
	struct Spectrum_Image_Texture final: Spectrum_Texture {
		Spectrum_Image_Texture(std::unique_ptr<image::Image> image);
		auto sample(Context const& ctx) -> Element;

	private:
		std::unique_ptr<image::Image> image;
	};
}
