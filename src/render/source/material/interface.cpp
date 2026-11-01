#include <metatron/render/material/interface.hpp>

namespace mtt::render {

auto Interface_Bsdf::operator()(
    fv3 const& wo, fv3 const& wi, f32 u
) const noexcept -> Bsdf_Interaction {
    return {fv4{1.f}, fv4{1.f}, wo, 1.f};
}

auto Interface_Bsdf::sample(
    core::Ray_Context const& ctx, fv3 const& u
) const noexcept -> Bsdf_Interaction {
    return {fv4{1.f}, fv4{1.f}, ctx.r.d, 1.f};
}

auto Interface_Bsdf::flags() const noexcept -> Bsdf_Flags {
    return Bsdf_Flags::bsdf_interface;
}

auto Interface_Material::sample(
    core::Ray_Context const& ctx,
    fv2 const& uv, fv4 const& duvdxy
) const noexcept -> Material_Interaction<render::Interface_Bsdf> {
    return {
        .bsdf = render::Interface_Bsdf{},
        .emission = fv4{0.f},
        .normal = {0.f, 0.f, 1.f},
        .degraded = false,
    };
}

auto Interface_Material::flags() const noexcept -> Material_Flags {
    return Material_Flags::material_interface;
}

}
