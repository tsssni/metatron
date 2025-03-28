#include <metatron/core/spectra/visible.hpp>

namespace metatron::spectra {
	Visible_Spectrum::Visible_Spectrum(std::array<f32, visible_range>&& data): data(std::move(data)) {}

	auto Visible_Spectrum::operator()(f32 lambda) const -> f32 {
		auto idx = usize(std::clamp(std::floor(lambda), visible_lambda[0], visible_lambda[1]));
		return data[idx];
	}
}
