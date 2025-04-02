#include <metatron/geometry/material/texture/spectrum/image.hpp>
#include <metatron/core/spectra/rgb.hpp>

namespace metatron::material {
	Spectrum_Image_Texture::Spectrum_Image_Texture(std::unique_ptr<image::Image> image, color::Color_Space::Spectrum_Type spectrum_type)
		: image(std::move(image)), spectrum_type(spectrum_type) {}

	auto Spectrum_Image_Texture::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) -> Element {
		auto pos = coord.uv * image->size;
		auto pixel = math::Vector<f32, 4>{(*image)[pos[0], pos[1]]};
		auto spectrum = image->color_space->to_spectrum(pixel, spectrum_type);
		return ctx.L & *spectrum;
	}
}
