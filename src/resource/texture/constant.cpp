#include <metatron/resource/texture/constant.hpp>
#include <metatron/resource/spectra/rgb.hpp>

namespace mtt::texture {
    auto Constant_Spectrum_Texture::operator()(
        cref<image::Coordinate> coord, cref<fv4> spec
    ) const noexcept -> fv4 {
        return spec & x;
    }

    auto Constant_Spectrum_Texture::sample(
        cref<eval::Context> ctx, cref<fv2> u
    ) const noexcept -> fv2 {
        return u;
    }

    auto Constant_Spectrum_Texture::pdf(
        cref<fv2> uv
    ) const noexcept -> f32 {
        return 1.f;
    }

    auto Constant_Vector_Texture::operator()(
        cref<image::Coordinate> coord
    ) const noexcept -> fv4 {
        return x;
    }

    auto Constant_Vector_Texture::sample(
        cref<eval::Context> ctx, cref<fv2> u
    ) const noexcept -> fv2 {
        return u;
    }

    auto Constant_Vector_Texture::pdf(
        cref<fv2> uv
    ) const noexcept -> f32 {
        return 1.f;
    }
}
