#pragma once
#include <metatron/resource/light/interaction.hpp>
#include <metatron/resource/texture/texture.hpp>

namespace mtt::light {
    struct Environment_Light final {
        texture::Spectrum_Texture env_map;

        auto operator()(
            cref<math::Ray> r, cref<fv4> lambda
        ) const noexcept -> Interaction;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> Interaction;
        auto flags() const noexcept -> Flags;
    };
}
