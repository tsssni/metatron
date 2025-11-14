#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/resource/texture/texture.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::light {
    struct Environment_Light final {
        tag<texture::Spectrum_Texture> env_map;

        auto operator()(
            cref<math::Ray> r, cref<stsp> spec
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<eval::Context> ctx, cref<fv2> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
