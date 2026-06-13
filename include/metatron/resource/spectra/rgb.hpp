#pragma once
#include <metatron/resource/color/color-space.hpp>

namespace mtt::spectra {
    // method: https://jo.dreggn.org/home/2019_wide_gamut.pdf
    // polynomial fits data: https://github.com/mitsuba-renderer/mitsuba3/tree/master/ext/rgb2spec
    struct Rgb_Spectrum final {
        struct Descriptor final {
            fv3 c;
            color::Color_Space::Spectrum_Type type;
            color::proxy::Color_Space color_space = color::proxy::Color_Space::entity("/color-space/sRGB");
        };
        Rgb_Spectrum(cref<Descriptor> desc) noexcept;
        Rgb_Spectrum() noexcept = default;
        auto operator()(f32 lambda) const noexcept -> f32;

    private:
        fv3 c;
        f32 s;
        u32 illuminant;
    };
}
