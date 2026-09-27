#pragma once
#include <metatron/resource/spectra/spectrum.hpp>

namespace mtt::spectra {
    struct Stochastic_Spectrum final {
        fv4 lambda = {};
        fv4 value = {};

        Stochastic_Spectrum() noexcept = default;
        Stochastic_Spectrum(fv4 const& lambda, fv4 const& value) noexcept;
        Stochastic_Spectrum(f32 u, f32 v = 0.f) noexcept;

        auto operator()(Spectrum spectrum) const noexcept -> f32;
    };
}
