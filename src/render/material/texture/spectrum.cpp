#include <metatron/core/spectra/rgb.hpp>
#include <metatron/render/material/texture/spectrum.hpp>

namespace metatron::material {
	Spectrum_Image_Texture::Spectrum_Image_Texture(std::unique_ptr<image::Image> image)
		: image(std::move(image)) {}

	auto Spectrum_Image_Texture::operator()(math::Vector<f32, 2> const& uv) -> Element {
		auto pos = uv * image->size;
		auto pixel = math::Vector<f32, 4>{(*image)[pos[0], pos[1]]};
		auto rgb_spec = std::make_unique<spectra::Rgb_Spectrum>(pixel);
		return rgb_spec;
	}

	auto Spectrum_Image_Texture::sample(shape::Interaction const& intr) -> Element {
		auto pos = intr.uv * image->size;
		auto pixel = math::Vector<f32, 4>{(*image)[pos[0], pos[1]]};
		auto rgb_spec = std::make_unique<spectra::Rgb_Spectrum>(pixel);
		return rgb_spec;
	}
}
