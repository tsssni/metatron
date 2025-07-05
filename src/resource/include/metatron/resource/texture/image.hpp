#pragma once
#include <metatron/resource/image/image.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/texture/texture.hpp>

namespace mtt::texture {
	template<typename T>
	struct Image_Texture final: Texture<T> {};

	template<>
	struct Image_Texture<math::Vector<f32, 4>> final: Texture<math::Vector<f32, 4>> {
		std::vector<std::unique_ptr<image::Image>> images;

		Image_Texture(std::unique_ptr<image::Image> image);

		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> math::Vector<f32, 4>;
	};

	template<>
	struct Image_Texture<spectra::Stochastic_Spectrum> final: Texture<spectra::Stochastic_Spectrum> {
		color::Color_Space::Spectrum_Type spectrum_type;

		Image_Texture(
			std::unique_ptr<image::Image> image,
			color::Color_Space::Spectrum_Type spectrum_type
		);

		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const -> spectra::Stochastic_Spectrum;

	private:
		Image_Texture<math::Vector<f32, 4>> image_tex;
	};
}
