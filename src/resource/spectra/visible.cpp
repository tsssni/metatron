#include <metatron/resource/spectra/visible.hpp>
#include <cstring>

namespace mtt::spectra {
    Visible_Spectrum::Visible_Spectrum(std::span<f32, visible_range> data) noexcept {
        std::memcpy(storage.data(), data.data(), visible_range * sizeof(f32));
    }

    auto Visible_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        auto idx = math::clamp(usize(std::round(lambda) - visible_lambda[0]), 0uz, visible_range - 1uz);
        return storage[idx];
    }
}
