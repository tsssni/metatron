#include <metatron/resource/spectra/visible.hpp>

namespace mtt::spectra {
    Visible_Spectrum::Visible_Spectrum(std::array<f32, visible_range>&& data) noexcept
    : data(std::move(data)) {}

    auto Visible_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        auto idx = std::clamp(usize(std::round(lambda) - visible_lambda[0]), 0uz, visible_range - 1uz);
        return data[idx];
    }
}
