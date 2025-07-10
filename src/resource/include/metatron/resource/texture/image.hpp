#pragma once
#include <metatron/resource/image/image.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/texture/texture.hpp>

namespace mtt::texture {
	template<typename T>
	struct Image_Texture final: Texture<T> {};

	template<>
	struct Image_Texture<math::Vector<f32, 4>> final {
		std::vector<std::unique_ptr<image::Image>> images;

		Image_Texture(std::unique_ptr<image::Image> image) noexcept;

		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const noexcept -> math::Vector<f32, 4>;
	};

	template<>
	struct Image_Texture<spectra::Stochastic_Spectrum> final {
		color::Color_Space::Spectrum_Type spectrum_type;

		Image_Texture(
			std::unique_ptr<image::Image> image,
			color::Color_Space::Spectrum_Type spectrum_type
		) noexcept;

		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const noexcept -> spectra::Stochastic_Spectrum;

	private:
		Image_Texture<math::Vector<f32, 4>> image_tex;
	};
}
