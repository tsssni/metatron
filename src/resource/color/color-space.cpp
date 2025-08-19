#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::color {
	std::unordered_map<std::string, view<Color_Space>> Color_Space::color_spaces;

	Color_Space::Color_Space(
		math::Vector<f32, 2> const& r_chroma,
		math::Vector<f32, 2> const& g_chroma,
		math::Vector<f32, 2> const& b_chroma,
		view<spectra::Spectrum> illuminant,
		std::function<f32(f32)> encode,
		std::function<f32(f32)> decode,
		view<Scale> scale,
		view<Table> table
	): 
	illuminant(illuminant),
	scale(scale),
	table(table),
	encode(encode),
	decode(decode) {
		// project color primaries and white point to Y=1
		auto w = ~illuminant;
		w /= math::sum(w);
		w = xyY_to_XYZ({w[0], w[1], 1.f});
		auto r= xyY_to_XYZ({r_chroma, 1.f});
		auto g= xyY_to_XYZ({g_chroma, 1.f});
		auto b= xyY_to_XYZ({b_chroma, 1.f});

		auto rgb = math::transpose(math::Matrix<f32, 3, 3>{r, g, b});
		auto inv_rgb = math::inverse(rgb);
		auto c = inv_rgb | w;
		to_XYZ = rgb | math::Matrix<f32, 3, 3>{c[0], c[1], c[2]};
		from_XYZ = math::inverse(to_XYZ);
	}

	auto Color_Space::to_spectrum(math::Vector<f32, 3> rgb, Spectrum_Type type) const -> poly<spectra::Spectrum> {
		if (false
		|| math::any([](f32 x, usize i){ return x < 0.f; }, rgb)
		|| math::any([](f32 x, usize i){ return x > 1.f; }, rgb)) {
			assert("RGB exceed [0.0, 1.0]");
		}

		auto s = 0.f;
		auto w = type == Color_Space::Spectrum_Type::illuminant ? illuminant : nullptr;
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
		rgb = rgb / s;

		if (rgb[0] == rgb[1] && rgb[1] == rgb[2]) {
			return make_poly<spectra::Spectrum, spectra::Rgb_Spectrum>(
				math::Vector<f32, 3>{
					(rgb[0] - 0.5f) / math::sqrt(rgb[0] * (1.f - rgb[0])),
					0.f, 0.f,
				},
				s, w
			);
		}

		auto maxc = math::maxi(rgb);
		auto z = rgb[maxc];
		auto x = rgb[(maxc + 1) % 3] * (table_res - 1) / z;
		auto y = rgb[(maxc + 2) % 3] * (table_res - 1) / z;

		// compute integer indices and offsets for coefficient interpolation
		auto xi = std::min((i32)x, table_res - 2);
		auto yi = std::min((i32)y, table_res - 2);
		auto zi = std::clamp(i32(std::lower_bound(std::begin(*scale), std::end(*scale), z) - std::begin(*scale)) - 1, 0, table_res - 2);
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

			c[2 - i] = math::lerp(
				math::lerp(
					math::lerp(co(0, 0, 0), co(1, 0, 0), dx),
					math::lerp(co(0, 1, 0), co(1, 1, 0), dx),
					dy
				),
				math::lerp(
					math::lerp(co(0, 0, 1), co(1, 0, 1), dx),
					math::lerp(co(0, 1, 1), co(1, 1, 1), dx),
					dy
				),
				dz
			);
		}

		return make_poly<spectra::Spectrum, spectra::Rgb_Spectrum>(c, s, w);
	}
}
