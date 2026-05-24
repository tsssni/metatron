#pragma once

namespace mtt::spectra {
    struct Blackbody_Spectrum final {
        f32 T;
        auto operator()(f32 lambda) const noexcept -> f32;
    };
}
