#include <metatron/render/photo/film.hpp>
#include <metatron/core/math/constant.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::photo {
    Fixel::Fixel(
        mut<Film> film,
        math::Vector<usize, 2> const& pixel,
        math::Vector<f32, 2> const& position,
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

    auto Fixel::operator=(spectra::Stochastic_Spectrum const& spectrum) noexcept -> void {
        auto xyz = math::Vector<f32, 3>{
            spectrum(film->r),
            spectrum(film->g),
            spectrum(film->b),
        };
        auto rgb = film->color_space->from_XYZ | xyz;
        (*film->image)[pixel[0], pixel[1]] += {rgb * weight, weight};
    }

    Film::Film(Descriptor const& desc) noexcept:
    spp(desc.spp), depth(desc.depth),
    film_size(desc.film_size),
    dxdy(desc.film_size / desc.image_size),
    r(desc.r), g(desc.g), b(desc.b) {
        film_size = desc.film_size;
        auto aspect_ratio = f32(desc.image_size[0]) / f32(desc.image_size[1]);
        if (aspect_ratio > 1.f) film_size[1] = film_size[0] / aspect_ratio;
        else film_size[0] = film_size[1] * aspect_ratio;
        dxdy = desc.film_size / desc.image_size;

        auto img = image::Image{};
        img.size = {desc.image_size, 4, 4};
        img.linear = true;
        img.pixels.resize(1);
        img.pixels.front().resize(math::prod(img.size));

        auto& vec = stl::vector<image::Image>::instance();
        auto lock = vec.lock();
        image = vec.push_back(std::move(img));
    }

    auto Film::operator()(
        view<filter::Filter> filter,
        math::Vector<usize, 2> const& pixel,
        math::Vector<f32, 2> const& u
    ) noexcept -> Fixel {
        auto f_intr = *filter->sample(u);
        auto pixel_position = math::Vector<f32, 2>{pixel} + 0.5f + f_intr.p;
        auto uv = pixel_position / image->size;
        auto film_position = (uv - 0.5f) * math::Vector<f32, 2>{-1.f, 1.f} * film_size;

        return {
            this,
            pixel,
            film_position,
            math::guarded_div(f_intr.weight, f_intr.pdf)
        };
    }
}
