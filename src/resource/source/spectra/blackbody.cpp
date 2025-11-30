#include <metatron/resource/spectra/blackbody.hpp>
#include <metatron/core/math/arithmetic.hpp>

namespace mtt::spectra {
    auto Blackbody_Spectrum::operator()(f32 lambda) const noexcept -> f32 {
        if (T <= 0.f) return 0.f;

        auto constexpr c = 299792458.f;
        auto constexpr h = 6.62606957e-34f;
        auto constexpr kb = 1.3806488e-23f;

        auto l = lambda * 1e-9f;
        auto L = (2.f * h * c * c) / (math::pow(l, 5) * (std::exp((h * c) / (l * kb * T)) - 1.f));
        return std::isnan(L) ? 0.f : L;
    }
}
