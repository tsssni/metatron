#pragma once
#include <metatron/resource/material/material.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::material {
    struct Physical_Material final {
        stl::proxy<texture::Spectrum_Texture> reflectance;
        stl::proxy<texture::Spectrum_Texture> eta;
        stl::proxy<texture::Spectrum_Texture> k;
        stl::proxy<texture::Spectrum_Texture> emission;

        stl::proxy<texture::Vector_Texture> alpha;
        stl::proxy<texture::Vector_Texture> alpha_u;
        stl::proxy<texture::Vector_Texture> alpha_v;
        stl::proxy<texture::Vector_Texture> normal;

        auto sample (
            eval::Context const& ctx,
            image::Coordinate const& coord
        ) const noexcept -> std::optional<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
