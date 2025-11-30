#pragma once
#include <metatron/render/filter/filter.hpp>
#include <metatron/render/scene/entity.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/spectra/color-space.hpp>
#include <metatron/resource/opaque/image.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::photo {
    struct Film;

    struct Fixel final {
        uzv2 pixel;
        fv2 position;
        fv2 dxdy;
        f32 weight;

        Fixel(
            mut<Film> film,
            cref<uzv2> pixel,
            cref<fv2> position,
            f32 weight
        ) noexcept;

        auto operator=(cref<spectra::Stochastic_Spectrum> spectrum) noexcept -> void;

    private:
        mut<Film> film;
    };

    struct Film final {
        usize spp;
        usize depth;
        fv2 film_size;
        fv2 dxdy;
        tag<opaque::Image> image;
        tag<spectra::Color_Space> color_space;

        struct Descriptor final {
            usize spp = 16uz;
            usize depth = 64uz;
            fv2 film_size = {0.036f, 0.024f};
            fv2 image_size = {1280uz, 720uz};
            tag<spectra::Spectrum> r = spectra::Spectrum::spectra["CIE-X"];
            tag<spectra::Spectrum> g = spectra::Spectrum::spectra["CIE-Y"];
            tag<spectra::Spectrum> b = spectra::Spectrum::spectra["CIE-Z"];
            tag<spectra::Color_Space> color_space = spectra::Color_Space::color_spaces["sRGB"];
        };
        Film(cref<Descriptor> desc) noexcept;

        auto operator()(
            view<filter::Filter> filter,
            cref<uzv2> pixel,
            cref<fv2> u
        ) noexcept -> Fixel;

    private:
        tag<spectra::Spectrum> r;
        tag<spectra::Spectrum> g;
        tag<spectra::Spectrum> b;
        friend Fixel;
    };
}
