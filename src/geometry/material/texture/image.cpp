#include <metatron/geometry/material/texture/image.hpp>
#include <metatron/core/spectra/rgb.hpp>
#include <bit>

namespace metatron::material {
	Image_Texture<spectra::Stochastic_Spectrum>::Image_Texture(
		std::unique_ptr<image::Image> image,
		color::Color_Space::Spectrum_Type spectrum_type,
		usize mips
	): spectrum_type(spectrum_type) {
		auto size = math::Vector<usize, 2>{image->size};
		auto channels = image->size[2];
		auto stride = image->size[3];

		auto max_mips = usize(std::bit_width(math::max(size)));
		if (mips > 0) {
			max_mips = std::min(max_mips, mips);
		}

		images.reserve(max_mips);
		images.push_back(std::move(image));

		for (auto mip = 1uz; mip < max_mips; mip++) {
			size[0] = std::max(1uz, size[0] >> 1uz);
			size[1] = std::max(1uz, size[1] >> 1uz);

			images.push_back(std::make_unique<image::Image>(
				math::Vector<usize, 4>{size, channels, stride},
				images.front()->color_space,
				images.front()->linear
			));
			auto& image = *images[mip];
			auto& upper = *images[mip - 1];

			for (auto j = 0uz; j < size[1]; j++) {
				for (auto i = 0uz; i < size[0]; i++) {
					image[i, j] = 0.25f * (math::Vector<f32, 4>{0.f}
						+ math::Vector<f32, 4>{upper[i * 2uz + 0, j * 2uz + 0]}
						+ math::Vector<f32, 4>{upper[i * 2uz + 0, j * 2uz + 1]}
						+ math::Vector<f32, 4>{upper[i * 2uz + 1, j * 2uz + 0]}
						+ math::Vector<f32, 4>{upper[i * 2uz + 1, j * 2uz + 1]}
					);
				}
			}
		}
	}

	auto Image_Texture<spectra::Stochastic_Spectrum>::sample(
		eval::Context const& ctx,
		Coordinate const& coord
	) const -> spectra::Stochastic_Spectrum {
		auto mip = std::min(0uz, images.size() - 1uz);
		auto& image = *images[mip];
		auto pos = coord.uv * image.size;
		auto pixel = math::Vector<f32, 4>{image[pos[0], pos[1]]};
		auto spectrum = image.color_space->to_spectrum(pixel, spectrum_type);
		return ctx.L & *spectrum;
	}
}
