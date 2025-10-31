#pragma once

namespace mtt::spectra {
    struct Constant_Spectrum final {
        f32 x;
        auto operator()(f32 lambda) const noexcept -> f32;
    };
}
