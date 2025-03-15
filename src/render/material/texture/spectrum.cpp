#include <metatron/core/spectra/rgb.hpp>
#include <metatron/render/material/texture/spectrum.hpp>

namespace metatron::material {
	Spectrum_Image_Texture::Spectrum_Image_Texture(std::unique_ptr<image::Image> image)
		: image(std::move(image)) {}

	auto Spectrum_Image_Texture::operator()(math::Vector<f32, 2> const& uv) -> std::optional<Interaction> {
		auto pos = math::Vector<f32, 2>{uv[0] * image->size[0], uv[1] * image->size[1]};
		auto pixel = math::Vector<f32, 4>{(*image)[pos[0], pos[1]]};
		auto rgb_spec = std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{pixel[0], pixel[1], pixel[2]});
		return Interaction{std::move(rgb_spec)};
	}

	auto Spectrum_Image_Texture::sample(shape::Interaction const& intr) -> std::optional<Interaction> {
		auto pos = math::Vector<f32, 2>{intr.uv[0] * image->size[0], intr.uv[1] * image->size[1]};
		auto pixel = math::Vector<f32, 4>{(*image)[pos[0], pos[1]]};
		auto rgb_spec = std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{pixel[0], pixel[1], pixel[2]});
		return Interaction{std::move(rgb_spec)};
	}
}
