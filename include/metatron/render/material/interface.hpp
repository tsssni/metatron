#pragma once
#include <metatron/render/material/interaction.hpp>

namespace mtt::render {

struct Interface_Bsdf final {
    auto operator()(
        fv3 const& wo, fv3 const& wi, f32 u
    ) const noexcept -> Bsdf_Interaction;
    auto sample(
        core::Ray_Context const& ctx, fv3 const& u
    ) const noexcept -> Bsdf_Interaction;
    auto flags() const noexcept -> Bsdf_Flags;
};

struct [[mtt::reflection]] Interface_Material final {
    auto sample(
        core::Ray_Context const& ctx,
        fv2 const& uv, fv4 const& duvdxy
    ) const noexcept -> Material_Interaction<Interface_Bsdf>;
    auto flags() const noexcept -> Material_Flags;
};

}
