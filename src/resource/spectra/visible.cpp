#include <metatron/resource/spectra/visible.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::spectra {
	Visible_Spectrum::Visible_Spectrum(std::array<f32, visible_range>&& data) noexcept
	: data(std::move(data)) {}

	auto Visible_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
		auto idx = std::clamp(usize(std::round(lambda) - visible_lambda[0]), 0uz, visible_range - 1uz);
		return data[idx];
	}

	auto Visible_Spectrum::from_spectrum(view<Spectrum> spec, bool normalize) noexcept -> Visible_Spectrum {
		auto d = normalize ? CIE_Y_integral / (Spectrum::spectra["CIE-Y"] | spec) : 1.f;
		auto data = std::array<f32, visible_range>{};
		for (auto i = visible_lambda[0]; i <= visible_lambda[1]; i++) {
			data[usize(i - visible_lambda[0])] = (*spec)(i) * d;
		}
		return Visible_Spectrum{std::move(data)};
	}
}
