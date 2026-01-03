#pragma once
#include <metatron/render/filter/filter.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/spectra/color-space.hpp>
#include <metatron/resource/muldim/image.hpp>
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
        // opaque type are not allowed in shader struct,
        // and film image is not included in bindless sampled images,
        // so use static to make it external to film struct.
        muldim::Image static image;

        u32 spp;
        u32 depth;
        u32 stride;
        fv2 film_size;
        fv2 dxdy;
        tag<spectra::Color_Space> color_space;

        struct Descriptor final {
            u32 spp = 16;
            u32 depth = 64;
            u32 stride = 1;
            fv2 film_size = {0.036f, 0.024f};
            uv2 image_size = {1280, 720};
            tag<spectra::Spectrum> r = entity<spectra::Spectrum>("/spectrum/CIE-X");
            tag<spectra::Spectrum> g = entity<spectra::Spectrum>("/spectrum/CIE-Y");
            tag<spectra::Spectrum> b = entity<spectra::Spectrum>("/spectrum/CIE-Z");
            tag<spectra::Color_Space> color_space = entity<spectra::Color_Space>("/color-space/sRGB");
        };
        Film(cref<Descriptor> desc) noexcept;
        Film() noexcept = default;

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
