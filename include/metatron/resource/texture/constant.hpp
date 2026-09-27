#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/muldim/image.hpp>

namespace mtt::texture {
    struct Constant_Spectrum_Texture final {
        spectra::Spectrum x;

        auto operator()(
            muldim::Coordinate const& coord, fv4 const& spec
        ) const noexcept -> fv4;
        auto sample(
            math::Context const& ctx, fv2 const& u
        ) const noexcept -> fv2;
        auto pdf(fv2 const& uv) const noexcept -> f32;
    };

    struct Constant_Vector_Texture final {
        fv4 x;

        auto operator()(
            muldim::Coordinate const& coord
        ) const noexcept -> fv4;
        auto sample(
            math::Context const& ctx, fv2 const& u
        ) const noexcept -> fv2;
        auto pdf(fv2 const& uv) const noexcept -> f32;
    };
}
