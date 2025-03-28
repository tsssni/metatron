#include <metatron/core/color/color-space.hpp>

namespace metatron::color {
	extern color::Color_Space::Scale sRGB_spectrum_z;
	extern color::Color_Space::Table sRGB_spectrum_table;

	std::unique_ptr<Color_Space> Color_Space::sRGB;

	Color_Space::Color_Space(
		math::Vector<f32, 2> const& r_chroma,
		math::Vector<f32, 2> const& g_chroma,
		math::Vector<f32, 2> const& b_chroma,
		spectra::Spectrum const* white_point,
		Scale const* scale,
		Table const* table
	): white_point(white_point), scale(scale), table(table) {
		auto w = &(*white_point);
		auto r= math::Vector<f32, 3>{r_chroma, 1.f - r_chroma[0] - r_chroma[1]};
		auto g= math::Vector<f32, 3>{g_chroma, 1.f - g_chroma[0] - g_chroma[1]};
		auto b= math::Vector<f32, 3>{b_chroma, 1.f - b_chroma[0] - b_chroma[1]};

		auto rgb = math::Matrix<f32, 3, 3>{
			{r[0], g[0], b[0]},
			{r[1], g[1], b[1]},
			{r[2], g[2], b[2]},
		};
		auto inv_rgb = math::inverse(rgb);
		auto c = inv_rgb | w;
		to_XYZ = rgb * math::Matrix<f32, 3, 3>{c[0], c[1], c[2]};
		from_XYZ = math::inverse(to_XYZ);
	}

	auto Color_Space::initialize() -> void {
		sRGB = std::make_unique<Color_Space>(
			math::Vector<f32, 2>{0.64f, 0.33f},
			math::Vector<f32, 2>{0.30f, 0.60f},
			math::Vector<f32, 2>{0.15f, 0.06f},
			spectra::Spectrum::CIE_D65.get(),
			&sRGB_spectrum_z,
			&sRGB_spectrum_table
		);
	}
}
