#include <metatron/resource/spectra/visible.hpp>

namespace mtt::spectra {
	Visible_Spectrum::Visible_Spectrum(std::array<f32, visible_range>&& data): data(std::move(data)) {}

	auto Visible_Spectrum::operator()(f32 lambda) const -> f32 {
		auto idx = usize(std::clamp(std::floor(lambda), visible_lambda[0], visible_lambda[1]));
		auto frac = lambda - idx;
		idx -= usize(visible_lambda[0]);
		return math::lerp(data[idx], data[idx + 1], frac);
	}
}
