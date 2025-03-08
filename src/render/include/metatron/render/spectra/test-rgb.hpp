#pragma once
#include <metatron/render/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>

namespace metatron::spectra {
	struct Test_Rgb_Spectrum final: Spectrum {
		mutable f32 min_lambda;
		mutable f32 max_lambda;

		Test_Rgb_Spectrum(f32 min_lambda, f32 max_lambda);

		auto operator()(f32 lambda) const -> f32;
		auto operator()(Spectrum const& spectrum) const -> f32;
		auto operator*(Spectrum const& spectrum) const -> Spectrum const&;
	};
}
