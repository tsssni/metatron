#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::texture {
    struct Constant_Spectrum_Texture final {
        tag<spectra::Spectrum> x;

        auto operator()(
            cref<image::Coordinate> coord, cref<fv4> spec
        ) const noexcept -> fv4;
        auto sample(
            cref<eval::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;
    };

    struct Constant_Vector_Texture final {
        fv4 x;

        auto operator()(
            cref<image::Coordinate> coord
        ) const noexcept -> fv4;
        auto sample(
            cref<eval::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;
    };
}
