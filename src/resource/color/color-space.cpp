#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace metatron::color {
	extern color::Color_Space::Scale sRGB_spectrum_z;
	extern color::Color_Space::Table sRGB_spectrum_table;

	std::unique_ptr<Color_Space> Color_Space::sRGB;

	Color_Space::Color_Space(
		math::Vector<f32, 2> const& r_chroma,
		math::Vector<f32, 2> const& g_chroma,
		math::Vector<f32, 2> const& b_chroma,
		spectra::Spectrum const* white_point,
		std::function<f32(f32)> encode,
		std::function<f32(f32)> decode,
		Scale const* scale,
		Table const* table
	): 
	white_point(white_point),
	scale(scale),
	table(table),
	encode(encode), 
	decode(decode) {
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
		to_XYZ = rgb | math::Matrix<f32, 3, 3>{c[0], c[1], c[2]};
		from_XYZ = math::inverse(to_XYZ);
	}

	auto Color_Space::to_spectrum(math::Vector<f32, 3> rgb, Spectrum_Type type) const -> std::unique_ptr<spectra::Spectrum> {
		if (rgb < math::Vector<f32, 3>{0.f} || rgb > math::Vector<f32, 3>{1.f}) {
			assert("RGB exceed [0.0, 1.0]");
		}

		auto s = 0.f;
		auto w = type == Color_Space::Spectrum_Type::illuminant ? white_point : nullptr;
		switch (type) {
			case Spectrum_Type::albedo:
				s = 1.f;
				break;
			case Spectrum_Type::unbounded:
				s = 2.f;
				break;
			case Spectrum_Type::illuminant:
				s = 2.f * std::max(rgb[0], std::max(rgb[1], rgb[2]));
				break;
		}
		rgb = math::guarded_div(rgb, s);

		if (rgb[0] == rgb[1] && rgb[1] == rgb[2]) {
			return std::make_unique<spectra::Rgb_Spectrum>(math::Vector<f32, 3>{0.f, 0.f, (rgb[0] - 0.5f) / math::sqrt(rgb[0] * (1.f - rgb[0]))}, s, w);
		}

		auto maxc = (rgb[0] > rgb[1]) ? ((rgb[0] > rgb[2]) ? 0 : 2) : ((rgb[1] > rgb[2]) ? 1 : 2);
		auto z = rgb[maxc];
		auto x = rgb[(maxc + 1) % 3] * (table_res - 1) / z;
		auto y = rgb[(maxc + 2) % 3] * (table_res - 1) / z;

		// compute integer indices and offsets for coefficient interpolation
		auto xi = std::max(0, std::min((i32)x, table_res - 2));
		auto yi = std::max(0, std::min((i32)y, table_res - 2));
		auto zi = std::max(0, std::min(table_res - 2, i32(std::lower_bound(std::begin(*scale), std::end(*scale), z) - std::begin(*scale) - 1)));
		auto dx = x - xi;
		auto dy = y - yi;
		auto dz = (z - (*scale)[zi]) / ((*scale)[zi + 1] - (*scale)[zi]);

		// trilinearly interpolate sigmoid polynomial coefficients
		auto c = math::Vector<f32, 3>{};
		for (auto i = 0; i < 3; i++) {
			// define co lambda for looking up sigmoid polynomial coefficients
			auto co = [&](int dx, int dy, int dz) {
				return (*table)[maxc][zi + dz][yi + dy][xi + dx][i];
			};

			c[2 - i] = std::lerp(
				std::lerp(
					std::lerp(co(0, 0, 0), co(1, 0, 0), dx),
					std::lerp(co(0, 1, 0), co(1, 1, 0), dx),
					dy
				),
				std::lerp(
					std::lerp(co(0, 0, 1), co(1, 0, 1), dx),
					std::lerp(co(0, 1, 1), co(1, 1, 1), dx),
					dy
				),
				dz
			);
		}

		return std::make_unique<spectra::Rgb_Spectrum>(c, s, w);
	}

	auto Color_Space::initialize() -> void {
		sRGB = std::make_unique<Color_Space>(
			math::Vector<f32, 2>{0.64f, 0.33f},
			math::Vector<f32, 2>{0.30f, 0.60f},
			math::Vector<f32, 2>{0.15f, 0.06f},
			spectra::Spectrum::CIE_D65.get(),
			[](f32 x) -> f32 {
				if (x <= 0.0031308f) {
					return 12.92f * x;
				} else {
					return 1.055f * std::pow(x, 1.f / 2.4f) - 0.055f;
				}
			},
			[](f32 x) -> f32 {
				if (x <= 0.04045f) {
					return x / 12.92f;
				} else {
					return pow((x + 0.055f) / 1.055f, 2.4f);
				}
			},
			&sRGB_spectrum_z,
			&sRGB_spectrum_table
		);
	}
}
