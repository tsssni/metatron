#pragma once
#include <metatron/resource/material/interaction.hpp>

namespace mtt::material {
    struct Physical_Material final {
        texture::Spectrum_Texture reflectance;
        texture::Spectrum_Texture eta;
        texture::Spectrum_Texture k;
        texture::Spectrum_Texture emission;

        texture::Vector_Texture alpha;
        texture::Vector_Texture alpha_u;
        texture::Vector_Texture alpha_v;
        texture::Vector_Texture normal;

        auto sample(
            cref<math::Context> ctx,
            cref<muldim::Coordinate> coord
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
