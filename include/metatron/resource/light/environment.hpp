#pragma once
#include <metatron/resource/light/interaction.hpp>
#include <metatron/resource/texture/texture.hpp>

namespace mtt::light {
    struct Environment_Light final {
        texture::Spectrum_Texture env_map;

        auto operator()(
            math::Ray const& r, fv4 const& lambda
        ) const noexcept -> Interaction;
        auto sample(
            math::Context const& ctx, fv2 const& u
        ) const noexcept -> Interaction;
        auto flags() const noexcept -> Flags;
    };
}
