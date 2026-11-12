#pragma once
#include <metatron/render/spectra/spectrum.hpp>

namespace mtt::render {

struct [[mtt::reflection(CIE_Y = "/spectra/CIE-Y")]] Checkerboard_Texture final {
    Spectrum x;
    Spectrum y;
    uv2 uv_scale;
    f32 w_x;
    f32 w_y;

    Checkerboard_Texture(
        Spectrum x,
        Spectrum y,
        Spectrum CIE_Y,
        uv2 uv_scale = {1}
    ) noexcept;

    auto operator()(
        fv2 const& uv, fv4 const& duvdxy, fv4 const& lambda
    ) const noexcept -> fv4;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> fv2;
    auto pdf(fv2 const& uv) const noexcept -> f32;
};

}
