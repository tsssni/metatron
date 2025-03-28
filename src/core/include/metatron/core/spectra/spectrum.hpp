#pragma once
#include <metatron/core/math/vector.hpp>
#include <memory>

namespace metatron::spectra {
	auto constexpr visible_lambda = math::Vector<f32, 2>{360.f, 830.f};

	struct Spectrum {
		virtual ~Spectrum() {}
		auto virtual operator()(f32 lambda) const -> f32 = 0;

		std::unique_ptr<Spectrum> static CIE_X;
		std::unique_ptr<Spectrum> static CIE_Y;
		std::unique_ptr<Spectrum> static CIE_Z;
		std::unique_ptr<Spectrum> static CIE_D65;

		auto static initialize() -> void;
	};

	auto inline operator|(Spectrum const& x, Spectrum const& y) -> f32 {
		auto integral = 0.f;
		for (auto lambda = visible_lambda[0]; lambda <= visible_lambda[1]; lambda++) {
			integral += x(lambda) * y(lambda);
		}
		return integral;
	}
}
