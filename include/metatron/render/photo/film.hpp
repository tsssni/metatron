#pragma once
#include <metatron/render/filter/filter.hpp>
#include <metatron/render/spectra/stochastic.hpp>

namespace mtt::render {

struct Film;

struct Fixel final {
    Film* film;
    uzv2 pixel;
    fv2 position;
    fv2 dxdy;
    f32 weight;

    auto operator=(render::Stochastic_Spectrum const& spectrum) noexcept -> void;
};

struct Film final {
    u32 spp;
    u32 depth;
    u32 stride;
    fv2 film_size;

    render::Spectrum r;
    render::Spectrum g;
    render::Spectrum b;
    render::Color_Space const* color_space;

    frw2 image;
    fv2 dxdy;
    auto update(uv2 image_size) noexcept -> void;

    auto operator()(
        render::Filter filter,
        uzv2 const& pixel,
        fv2 const& u
    ) noexcept -> Fixel;
};

}
