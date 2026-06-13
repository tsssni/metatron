#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/muldim/image.hpp>

namespace mtt::texture {
    struct Checkerboard_Texture final {
        struct Descriptor final {
            spectra::Spectrum x;
            spectra::Spectrum y;
            uv2 uv_scale = uv2{1};
        };
        Checkerboard_Texture(cref<Descriptor> desc) noexcept;
        Checkerboard_Texture() noexcept = default;

        auto operator()(
            cref<muldim::Coordinate> coord, cref<fv4> lambda
        ) const noexcept -> fv4;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;

    private:
        spectra::Spectrum x;
        spectra::Spectrum y;
        uv2 uv_scale;

        f32 w_x;
        f32 w_y;
    };
}
