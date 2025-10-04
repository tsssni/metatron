#pragma once

namespace mtt::spectra {
    struct Blackbody_Spectrum final {
        f32 T;
        Blackbody_Spectrum(f32 T) noexcept;
        auto operator()(f32 lambda) const noexcept -> f32;
    };
}
