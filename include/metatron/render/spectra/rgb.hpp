#pragma once
#include <metatron/render/spectra/color-space.hpp>

namespace mtt::render {

// method: https://jo.dreggn.org/home/2019_wide_gamut.pdf
// polynomial fits data: https://github.com/mitsuba-renderer/mitsuba3/tree/master/ext/rgb2spec
struct Rgb_Spectrum final {
    fv3 c;
    f32 s;
    Discrete_Spectrum* illuminant;
    Rgb_Spectrum(fv3 rgb, render::Color_Space::Spectrum_Type type, render::Color_Space const* color_space = nullptr) noexcept;
    auto operator()(f32 lambda) const noexcept -> f32;
};

}
