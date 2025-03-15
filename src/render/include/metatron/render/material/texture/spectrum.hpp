#pragma once
#include <metatron/core/image/image.hpp>
#include <metatron/core/spectra/spectrum.hpp>
#include <metatron/render/material/texture/texture.hpp>
#include <memory>

namespace metatron::material {
	struct Spectrum_Image_Texture final: Texture<std::unique_ptr<spectra::Spectrum>> {
		Spectrum_Image_Texture(std::unique_ptr<image::Image> image);
		auto operator()(math::Vector<f32, 2> const& uv) -> std::optional<Interaction>;
		auto sample(shape::Interaction const& intr) -> std::optional<Interaction>;

	private:
		std::unique_ptr<image::Image> image;
	};
}
