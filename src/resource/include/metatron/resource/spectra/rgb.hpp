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
            math::Vector<f32, 3> c;
            color::Color_Space::Spectrum_Type type;
            stl::proxy<color::Color_Space> color_space;
        };
        Rgb_Spectrum(Descriptor const& desc) noexcept;
        auto operator()(f32 lambda) const noexcept -> f32;

    private:
        std::array<f32, 3> c;
        f32 s;
        stl::proxy<Spectrum> illuminant;
    };
}
