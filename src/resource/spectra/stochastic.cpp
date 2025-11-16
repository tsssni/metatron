#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/core/math/distribution/spectrum.hpp>

namespace mtt::spectra {
    Stochastic_Spectrum::Stochastic_Spectrum(cref<fv4> lambda, cref<fv4> value) noexcept:
    lambda(lambda), value(value) {}

    Stochastic_Spectrum::Stochastic_Spectrum(f32 u, f32 v) noexcept {
        lambda = math::foreach([&](f32 l, usize i) {
            auto ui = math::mod(u + i / 4.f, 1.f);
            return math::Spectrum_Distribution{}.sample(ui);
        }, lambda);
        value = fv4{v};
    }

    auto Stochastic_Spectrum::operator()(view<Spectrum> s) const noexcept -> f32 {
        auto pdf = math::foreach([&](f32 l, usize i) {
            return math::Spectrum_Distribution{}.pdf(l);
        }, lambda);
        return math::sum(value * (lambda & s) / pdf) / 4.f;
    }

    auto Stochastic_Spectrum::operator()(tag<Spectrum> spectrum) const noexcept -> f32 {
        return (*this)(spectrum.data());
    }
}
