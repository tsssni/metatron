#pragma once
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::material {
    struct Physical_Material final {
        proxy<texture::Spectrum_Texture> reflectance;
        proxy<texture::Spectrum_Texture> eta;
        proxy<texture::Spectrum_Texture> k;
        proxy<texture::Spectrum_Texture> emission;

        proxy<texture::Vector_Texture> alpha;
        proxy<texture::Vector_Texture> alpha_u;
        proxy<texture::Vector_Texture> alpha_v;
        proxy<texture::Vector_Texture> normal;

        auto sample (
            eval::Context const& ctx,
            image::Coordinate const& coord
        ) const noexcept -> std::optional<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
