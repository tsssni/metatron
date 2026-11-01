#pragma once
#include <metatron/render/light/interaction.hpp>
#include <metatron/render/texture/texture.hpp>

namespace mtt::render {

struct Environment_Light final {
    render::Spectrum_Texture env_map;

    auto operator()(
        core::Ray const& r, fv4 const& lambda
    ) const noexcept -> Light_Interaction;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> Light_Interaction;
    auto flags() const noexcept -> Light_Flags;
};

}
