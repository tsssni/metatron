#pragma once
#include <metatron/render/filter/filter.hpp>
#include <metatron/render/scene/entity.hpp>
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/image/image.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::photo {
    struct Film;

    struct Fixel final {
        math::Vector<usize, 2> const pixel;
        math::Vector<f32, 2> const position;
        math::Vector<f32, 2> const dxdy;
        f32 const weight;

        Fixel(
            mut<Film> film,
            math::Vector<usize, 2> const& pixel,
            math::Vector<f32, 2> const& position,
            f32 weight
        ) noexcept;

        auto operator=(
            spectra::Stochastic_Spectrum const& spectrum
        ) noexcept -> void;

    private:
        mut<Film> film;
    };

    struct Film final {
        usize spp;
        usize depth;
        proxy<image::Image> image;
        math::Vector<f32, 2> film_size;
        math::Vector<f32, 2> dxdy;

        struct Descriptor final {
            usize spp = 16uz;
            usize depth = 64uz;
            math::Vector<f32, 2> film_size = {0.036f, 0.024f};
            math::Vector<f32, 2> image_size = {1280uz, 720uz};
            proxy<spectra::Spectrum> r = spectra::Spectrum::spectra["CIE-X"];
            proxy<spectra::Spectrum> g = spectra::Spectrum::spectra["CIE-Y"];
            proxy<spectra::Spectrum> b = spectra::Spectrum::spectra["CIE-Z"];
            proxy<color::Color_Space> color_space = color::Color_Space::color_spaces["sRGB"];
        };
        Film(Descriptor const& desc) noexcept;

        auto operator()(
            view<filter::Filter> filter,
            math::Vector<usize, 2> const& pixel,
            math::Vector<f32, 2> const& u
        ) noexcept -> Fixel;

    private:
        proxy<spectra::Spectrum> r;
        proxy<spectra::Spectrum> g;
        proxy<spectra::Spectrum> b;
        proxy<color::Color_Space> color_space;
        friend Fixel;
    };
}
