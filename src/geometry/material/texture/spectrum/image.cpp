#include <metatron/core/spectra/rgb.hpp>
#include <metatron/geometry/material/texture/spectrum/image.hpp>

namespace metatron::material {
	Spectrum_Image_Texture::Spectrum_Image_Texture(std::unique_ptr<image::Image> image)
		: image(std::move(image)) {}

	auto Spectrum_Image_Texture::sample(Context const& ctx) -> Element {
		auto pos = ctx.intr->uv * image->size;
		auto pixel = math::Vector<f32, 4>{(*image)[pos[0], pos[1]]};
		auto rgb_spec = std::make_unique<spectra::Rgb_Spectrum>(pixel);
		return rgb_spec;
	}
}
