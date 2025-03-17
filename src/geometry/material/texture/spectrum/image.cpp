#include <metatron/geometry/material/texture/spectrum/image.hpp>
#include <metatron/core/spectra/rgb.hpp>

namespace metatron::material {
	Spectrum_Image_Texture::Spectrum_Image_Texture(std::unique_ptr<image::Image> image)
		: image(std::move(image)) {}

	auto Spectrum_Image_Texture::sample(eval::Context const& ctx) -> Element {
		auto pos = *ctx.uv * image->size;
		auto pixel = math::Vector<f32, 4>{(*image)[pos[0], pos[1]]};
		return *ctx.L & spectra::Rgb_Spectrum{pixel};
	}
}
