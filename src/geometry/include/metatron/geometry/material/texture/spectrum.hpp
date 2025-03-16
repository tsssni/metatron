#pragma once
#include <metatron/core/image/image.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <metatron/geometry/material/texture/texture.hpp>
#include <memory>

namespace metatron::material {
	using Spectrum_Texture = Texture<std::unique_ptr<spectra::Spectrum>>;

	struct Spectrum_Image_Texture final: Spectrum_Texture {
		Spectrum_Image_Texture(std::unique_ptr<image::Image> image);
		auto operator()(math::Vector<f32, 2> const& uv) -> Element;
		auto sample(shape::Interaction const& intr) -> Element;

	private:
		std::unique_ptr<image::Image> image;
	};
}
