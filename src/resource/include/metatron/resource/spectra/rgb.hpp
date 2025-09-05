#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/polynomial.hpp>

namespace mtt::spectra {
	struct Rgb_Spectrum final {
		Rgb_Spectrum(
			math::Vector<f32, 3> const& c,
			f32 s = 1.f,
			view<Spectrum> illuminant = nullptr
		) noexcept;
		auto operator()(f32 lambda) const noexcept -> f32;

	private:
		std::array<f32, 3> c;
		f32 s;
		view<Spectrum> illuminant;
	};
}
