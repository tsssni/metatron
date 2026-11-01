#include <metatron/render/photo/film.hpp>

namespace mtt::render {

auto Fixel::operator=(render::Stochastic_Spectrum const& spectrum) noexcept -> void {
    auto xyz = fv3{
        spectrum(film->r),
        spectrum(film->g),
        spectrum(film->b),
    };
    auto rgb = film->color_space->from_XYZ | xyz;
    film->image[uzv3{pixel, 0}] += fv4{rgb * weight, weight};
}

auto Film::update(uv2 image_size) noexcept -> void {
    auto aspect_ratio = (f32)image_size[0] / (f32)image_size[1];
    if (aspect_ratio > 1.f) this->film_size[1] = film_size[0] / aspect_ratio;
    else this->film_size[0] = film_size[1] * aspect_ratio;
    image.update<core::ordinary<32, 32, 32, 32>, core::encoding::sf>({uzv2{image_size}, 1, 1});
    dxdy = film_size / image_size;
}

auto Film::operator()(
    render::Filter filter,
    uzv2 const& pixel,
    fv2 const& u
) noexcept -> Fixel {
    auto intr = filter.sample(u);
    auto pxpos = fv2{pixel} + 0.5f + intr.p;
    auto uv = pxpos / fv2{image.size()};
    auto fpos = (uv - 0.5f) * fv2{-1.f, 1.f} * film_size;
    auto dxdy = core::sign(fpos + this->dxdy - film_size / 2.f) * this->dxdy;

    return {
        this,
        pixel,
        fpos,
        dxdy,
        core::guarded_div(intr.weight, intr.pdf)
    };
}

}
