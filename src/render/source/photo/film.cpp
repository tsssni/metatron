#include <metatron/render/photo/film.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::photo {
    muldim::Image Film::image;

    Fixel::Fixel(
        mut<Film> film,
        cref<uzv2> pixel,
        cref<fv2> position,
        f32 weight
    ) noexcept:
    film(film),
    pixel(pixel),
    position(position),
    dxdy(math::foreach(
        [](f32 x, usize i){return x < 0.f ? 1.f : -1.f;},
        position + film->dxdy - film->film_size / 2.f
    ) * film->dxdy),
    weight(weight) {}

    auto Fixel::operator=(cref<spectra::Stochastic_Spectrum> spectrum) noexcept -> void {
        auto xyz = fv3{
            spectrum(film->r),
            spectrum(film->g),
            spectrum(film->b),
        };
        auto rgb = film->color_space->from_XYZ | xyz;
        film->image[pixel[0], pixel[1]] += {rgb * weight, weight};
    }

    Film::Film(cref<Descriptor> desc) noexcept:
    spp(desc.spp), depth(desc.depth), step(desc.step),
    film_size(desc.film_size),
    dxdy(desc.film_size / desc.image_size),
    r(desc.r), g(desc.g), b(desc.b),
    color_space(desc.color_space) {
        film_size = desc.film_size;
        auto aspect_ratio = f32(desc.image_size[0]) / f32(desc.image_size[1]);
        if (aspect_ratio > 1.f) film_size[1] = film_size[0] / aspect_ratio;
        else film_size[0] = film_size[1] * aspect_ratio;
        dxdy = desc.film_size / desc.image_size;

        image.size = {desc.image_size, 4, 4};
        image.linear = true;
        image.pixels.resize(1);
        image.pixels.front().resize(math::prod(image.size));
    }

    auto Film::operator()(
        view<filter::Filter> filter,
        cref<uzv2> pixel,
        cref<fv2> u
    ) noexcept -> Fixel {
        auto f_intr = *filter->sample(u);
        auto pixel_position = fv2{pixel} + 0.5f + f_intr.p;
        auto uv = pixel_position / image.size;
        auto film_position = (uv - 0.5f) * fv2{-1.f, 1.f} * film_size;

        return {
            this,
            pixel,
            film_position,
            math::guarded_div(f_intr.weight, f_intr.pdf)
        };
    }
}
