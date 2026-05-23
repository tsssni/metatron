#pragma once
#include <metatron/resource/spectra/spectrum.hpp>
#include <metatron/resource/muldim/image.hpp>
#include <metatron/core/math/eval.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::texture {
    struct Constant_Spectrum_Texture final {
        spectra::Spectrum x;

        auto operator()(
            cref<muldim::Coordinate> coord, cref<fv4> spec
        ) const noexcept -> fv4;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;
    };

    struct Constant_Vector_Texture final {
        fv4 x;

        auto operator()(
            cref<muldim::Coordinate> coord
        ) const noexcept -> fv4;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> fv2;
        auto pdf(cref<fv2> uv) const noexcept -> f32;
    };
}
