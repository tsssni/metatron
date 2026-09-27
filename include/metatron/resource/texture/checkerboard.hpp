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
        Checkerboard_Texture(Descriptor const& desc) noexcept;
        Checkerboard_Texture() noexcept = default;

        auto operator()(
            muldim::Coordinate const& coord, fv4 const& lambda
        ) const noexcept -> fv4;
        auto sample(
            math::Context const& ctx, fv2 const& u
        ) const noexcept -> fv2;
        auto pdf(fv2 const& uv) const noexcept -> f32;

    private:
        spectra::Spectrum x;
        spectra::Spectrum y;
        uv2 uv_scale;

        f32 w_x;
        f32 w_y;
    };
}
