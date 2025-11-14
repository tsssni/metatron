#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/core/math/vector.hpp>
#include <metatron/core/math/polynomial.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::spectra {
    // method: https://jo.dreggn.org/home/2019_wide_gamut.pdf
    // polynomial fits data: https://github.com/mitsuba-renderer/mitsuba3/tree/master/ext/rgb2spec
    struct Rgb_Spectrum final {
        struct Descriptor final {
            fv3 c;
            color::Color_Space::Spectrum_Type type;
            tag<color::Color_Space> color_space = color::Color_Space::color_spaces["sRGB"];
        };
        Rgb_Spectrum() noexcept = default;
        Rgb_Spectrum(cref<Descriptor> desc) noexcept;
        auto operator()(f32 lambda) const noexcept -> f32;

    private:
        std::array<f32, 3> c;
        f32 s;
        tag<Spectrum> illuminant;
    };
}
