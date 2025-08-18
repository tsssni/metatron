#include <metatron/resource/spectra/rgb.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::spectra {
	Rgb_Spectrum::Rgb_Spectrum(
		math::Vector<f32, 3> const& c,
		f32 s,
		view<Spectrum> illuminant
	) noexcept : polynomial({c[0], c[1], c[2]}), s(s), illuminant(illuminant) {}

	auto Rgb_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
		auto sigmoid = [](f32 x) -> f32 {
			if (std::isinf(x)) {
				return x < 0.f ? 0.f : 1.f;
			}
			return 0.5f + x / (2.f * math::sqrt(1.f + math::sqr(x)));
		};
		return s * sigmoid(polynomial(lambda)) * (illuminant ? (*illuminant)(lambda) : 1.f);
	}
}
