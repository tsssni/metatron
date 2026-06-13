#pragma once
#include <metatron/resource/spectra/spectrum.hpp>

namespace mtt::spectra {
    struct Stochastic_Spectrum final {
        fv4 lambda = {};
        fv4 value = {};

        Stochastic_Spectrum(cref<fv4> lambda, cref<fv4> value) noexcept;
        Stochastic_Spectrum(f32 u, f32 v = 0.f) noexcept;

        auto operator()(Spectrum spectrum) const noexcept -> f32;
    };
}
