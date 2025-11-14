#pragma once
#include <metatron/resource/texture/texture.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::texture {
    struct Checkerboard_Texture final {
        struct Descriptor final {
            tag<spectra::Spectrum> x;
            tag<spectra::Spectrum> y;
            uzv2 uv_scale = uzv2{1uz};
        };
        Checkerboard_Texture() noexcept = default;
        Checkerboard_Texture(cref<Descriptor> desc) noexcept;

        auto operator()(
            cref<image::Coordinate> coord, cref<stsp> spec
        ) const noexcept -> stsp;
        auto sample(
            cref<eval::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;

    private:
        tag<spectra::Spectrum> x;
        tag<spectra::Spectrum> y;
        uzv2 uv_scale;

        f32 w_x;
        f32 w_y;
    };
}
