#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>
#include <functional>

namespace mtt::color {
	auto constexpr table_res = 64;
	struct Color_Space final {
		using Scale = f32[table_res];
		using Table = f32[3][table_res][table_res][table_res][3];

		enum struct Spectrum_Type: usize {
			albedo,
			unbounded,
			illuminant,
		};

		math::Matrix<f32, 3, 3> from_XYZ;
		math::Matrix<f32, 3, 3> to_XYZ;
		view<spectra::Spectrum> white_point;
		std::function<f32(f32)> encode;
		std::function<f32(f32)> decode;

		Color_Space(
			math::Vector<f32, 2> const& r,
			math::Vector<f32, 2> const& g,
			math::Vector<f32, 2> const& b,
			view<spectra::Spectrum> white_point,
			std::function<f32(f32)> encode,
			std::function<f32(f32)> decode,
			view<Scale> scale,
			view<Table> table
		);

		auto to_spectrum(math::Vector<f32, 3> rgb, Spectrum_Type type) const -> poly<spectra::Spectrum>;

		poly<Color_Space> static sRGB;

		auto static initialize() -> void;

	private:
		Scale const* scale;
		Table const* table;
	};
}
