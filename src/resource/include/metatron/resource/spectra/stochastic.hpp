#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::spectra {
    struct Stochastic_Spectrum final {
        fv4 lambda = {};
        fv4 value = {};

        Stochastic_Spectrum(cref<fv4> lambda, cref<fv4> value) noexcept;
        Stochastic_Spectrum(f32 u, f32 v = 0.f) noexcept;

        auto operator()(view<Spectrum> spectrum) const noexcept -> f32;
        auto operator()(tag<Spectrum> spectrum) const noexcept -> f32;
    };
}
