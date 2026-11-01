#include <metatron/render/spectra/stochastic.hpp>
#include <metatron/core/distribution/spectrum.hpp>

namespace mtt::render {

Stochastic_Spectrum::Stochastic_Spectrum(fv4 const& lambda, fv4 const& value) noexcept
: lambda(lambda), value(value) {}

Stochastic_Spectrum::Stochastic_Spectrum(f32 u, f32 v) noexcept {
    auto sample = [&](f32 l, usize i) {
        auto ui = core::mod(u + i / 4.f, 1.f);
        return core::Spectrum_Distribution{}.sample(ui);
    };
    lambda = core::foreach(sample, lambda);
    value = fv4{v};
}

auto Stochastic_Spectrum::operator()(Spectrum s) const noexcept -> f32 {
    auto pdf = core::foreach([&](f32 l, auto) { return core::Spectrum_Distribution{}.pdf(l); }, lambda);
    return core::sum(value * (lambda & s) / pdf) / 4.f;
}

}
