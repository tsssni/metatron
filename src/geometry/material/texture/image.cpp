#include <metatron/geometry/material/texture/image.hpp>
#include <metatron/core/spectra/rgb.hpp>
#include <bit>

namespace metatron::material {
	Image_Texture<spectra::Stochastic_Spectrum>::Image_Texture(
		std::unique_ptr<image::Image> image,
		color::Color_Space::Spectrum_Type spectrum_type,
		usize mips
	): image(std::move(image)), spectrum_type(spectrum_type) {}

	auto Image_Texture<spectra::Stochastic_Spectrum>::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) const -> spectra::Stochastic_Spectrum {
		auto pos = coord.uv * image->size;
		auto pixel = math::Vector<f32, 4>{(*image)[pos[0], pos[1]]};
		auto spectrum = image->color_space->to_spectrum(pixel, spectrum_type);
		return ctx.L & *spectrum;
	}
}
