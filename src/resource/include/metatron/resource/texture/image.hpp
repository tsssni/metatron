#pragma once
#include <metatron/resource/image/image.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/texture/texture.hpp>

namespace mtt::texture {
	struct Image_Vector_Texture final {
		std::vector<poly<image::Image>> images;
		f32 anisotropy;

		Image_Vector_Texture(
			poly<image::Image> image,
			f32 anisotropy
		) noexcept;
		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const noexcept -> math::Vector<f32, 4>;

	private:
		auto mip(Coordinate const& coord, i32 lod) const noexcept -> math::Vector<f32, 4>;
		auto ewa(Coordinate const& coord, i32 lod) const noexcept -> math::Vector<f32, 4>;
	};

	struct Image_Spectrum_Texture final {
		color::Color_Space::Spectrum_Type type;
		Image_Spectrum_Texture(
			poly<image::Image> image,
			color::Color_Space::Spectrum_Type type,
			f32 anisotropy
		) noexcept;
		auto sample(
			eval::Context const& ctx,
			Coordinate const& coord
		) const noexcept -> spectra::Stochastic_Spectrum;

	private:
		Image_Vector_Texture image_tex;
	};
}
