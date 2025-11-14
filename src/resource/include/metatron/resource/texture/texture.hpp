#pragma once
#include <metatron/resource/shape/sphere.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/resource/image/image.hpp>

namespace mtt::texture {
    MTT_POLY_METHOD(texture_sample, sample);
    MTT_POLY_METHOD(texture_pdf, pdf);

    struct Spectrum_Texture final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        cref<image::Coordinate> coord, cref<stsp> spec
    ) const noexcept -> stsp>
    ::add_convention<texture_sample, auto (
        cref<eval::Context> ctx, cref<fv2> u
    ) const noexcept -> fv2>
    ::add_convention<texture_pdf, auto (cref<fv2> uv) const noexcept -> f32>
    ::template add_skill<pro::skills::as_view>
    ::build {};

    struct Vector_Texture final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        cref<image::Coordinate> coord
    ) const noexcept -> fv4>
    ::add_convention<texture_sample, auto (
        cref<eval::Context> ctx, fv2 u
    ) const noexcept -> fv2>
    ::add_convention<texture_pdf, auto (fv2 uv) const noexcept -> f32>
    ::template add_skill<pro::skills::as_view>
    ::build {};

    auto grad(
        cref<math::Ray_Differential> diff,
        cref<shape::Interaction> intr
    ) noexcept -> opt<image::Coordinate>;
}
