#include <metatron/resource/photo/film.hpp>
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
            spectrum(film->r.data()),
            spectrum(film->g.data()),
            spectrum(film->b.data()),
        };
        auto rgb = film->color_space->from_XYZ | xyz;
        (*film->image.data())[pixel[0], pixel[1]] += {rgb * weight, weight};
    }

    Film::Film(Descriptor const& desc) noexcept:
    film_size(desc.film_size),
    dxdy(desc.film_size / desc.image_size),
    filter(desc.filter),
    r(desc.r),
    g(desc.g),
    b(desc.b) {
        auto& vec = stl::poly_vector<device::Texture>::instance();
        image = vec.push_back<device::Texture>({});
        image->size = {desc.image_size, 4, 4};
        image->linear = true;
    }

    auto Film::operator()(
        math::Vector<usize, 2> const& pixel,
        math::Vector<f32, 2> const& u
    ) noexcept -> Fixel {
        auto f_intr = filter->sample(u).value();
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
