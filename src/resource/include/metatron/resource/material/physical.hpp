#pragma once
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::material {
    struct Physical_Material final {
        tag<texture::Spectrum_Texture> reflectance;
        tag<texture::Spectrum_Texture> eta;
        tag<texture::Spectrum_Texture> k;
        tag<texture::Spectrum_Texture> emission;

        tag<texture::Vector_Texture> alpha;
        tag<texture::Vector_Texture> alpha_u;
        tag<texture::Vector_Texture> alpha_v;
        tag<texture::Vector_Texture> normal;

        auto sample(
            cref<eval::Context> ctx,
            cref<image::Coordinate> coord
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
