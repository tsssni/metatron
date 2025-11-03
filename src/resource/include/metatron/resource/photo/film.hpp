#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/color/color-space.hpp>
#include <metatron/resource/image/image.hpp>
#include <metatron/core/math/filter/filter.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::photo {
    struct Camera;
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
        stl::proxy<image::Image> image;
        math::Vector<f32, 2> film_size;
        math::Vector<f32, 2> dxdy;

        struct Descriptor final {
            math::Vector<f32, 2> film_size;
            math::Vector<f32, 2> image_size;
            stl::proxy<math::Filter> filter;
            stl::proxy<spectra::Spectrum> r;
            stl::proxy<spectra::Spectrum> g;
            stl::proxy<spectra::Spectrum> b;
            stl::proxy<color::Color_Space> color_space;
        };
        Film(Descriptor const& desc) noexcept;

        auto operator()(
            math::Vector<usize, 2> const& pixel,
            math::Vector<f32, 2> const& u
        ) noexcept -> Fixel;

    private:
        stl::proxy<math::Filter> const filter;
        stl::proxy<spectra::Spectrum> const r;
        stl::proxy<spectra::Spectrum> const g;
        stl::proxy<spectra::Spectrum> const b;
        stl::proxy<color::Color_Space> const color_space;
        friend Fixel;
    };
}
