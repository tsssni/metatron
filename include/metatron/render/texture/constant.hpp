#pragma once
#include <metatron/render/spectra/spectrum.hpp>

namespace mtt::render {

struct [[mtt::reflection]] Constant_Spectrum_Texture final {
    Spectrum x;

    auto operator()(
        fv2 const& uv, fv4 const& duvdxy, fv4 const& spec
    ) const noexcept -> fv4;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> fv2;
    auto pdf(fv2 const& uv) const noexcept -> f32;
};

struct [[mtt::reflection]] Constant_Vector_Texture final {
    fv4 x;

    auto operator()(
        fv2 const& uv, fv4 const& duvdxy
    ) const noexcept -> fv4;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> fv2;
    auto pdf(fv2 const& uv) const noexcept -> f32;
};

}
