#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/spectra/color-space.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::spectra {
    // method: https://jo.dreggn.org/home/2019_wide_gamut.pdf
    // polynomial fits data: https://github.com/mitsuba-renderer/mitsuba3/tree/master/ext/rgb2spec
    struct Rgb_Spectrum final {
        struct Descriptor final {
            fv3 c;
            Color_Space::Spectrum_Type type;
            tag<Color_Space> color_space = entity<Color_Space>("/color-space/sRGB");
        };
        Rgb_Spectrum(cref<Descriptor> desc) noexcept;
        Rgb_Spectrum() noexcept = default;
        auto operator()(f32 lambda) const noexcept -> f32;

    private:
        fv3 c;
        f32 s;
        tag<Spectrum> illuminant;
    };
}
