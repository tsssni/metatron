#pragma once
#include <metatron/render/material/interaction.hpp>

namespace mtt::render {

struct Physical_Bsdf final {
    fv4 reflectance;
    fv4 fresnel_reflectance;
    fv4 eta;
    fv4 k;
    fv2 alpha;

    enum struct Type : u8 {
        lambertian,
        dielectric,
        conductive,
        plastic,
    } type;

    // microfacet:
    //    https://pbr-book.org/4ed/Reflection_Models/Roughness_Using_Microfacet_Theory
    // dielectric:
    //    https://pbr-book.org/4ed/Reflection_Models/Dielectric_BSDF
    //    https://pbr-book.org/4ed/Reflection_Models/Rough_Dielectric_BSDF
    // conductor:
    //    https://pbr-book.org/4ed/Reflection_Models/Conductor_BRDF
    // plastic:
    //    https://github.com/tunabrain/tungsten/blob/master/src/core/bsdfs/RoughPlasticBsdf.cpp
    //    https://tsssni.github.io/render/1733816209202-pbrt-v4-episode-9/#%E5%A1%91%E6%96%99bsdf
    auto operator()(
        fv3 const& wo, fv3 const& wi, f32 u
    ) const noexcept -> Bsdf_Interaction;
    auto sample(
        core::Ray_Context const& ctx, fv3 const& u
    ) const noexcept -> Bsdf_Interaction;
    auto flags() const noexcept -> Bsdf_Flags;
    auto degrade() noexcept -> bool;
};

auto init_physical_material() noexcept -> void;

struct Physical_Material final {
    render::Spectrum_Texture reflectance;
    render::Spectrum_Texture eta;
    render::Spectrum_Texture k;
    render::Spectrum_Texture emission;

    render::Vector_Texture alpha;
    render::Vector_Texture normal;
    buf<f32> static fresnel_reflectance_table;

    auto sample(
        core::Ray_Context const& ctx,
        fv2 const& uv, fv4 const& duvdxy
    ) const noexcept -> Material_Interaction<render::Physical_Bsdf>;
    auto flags() const noexcept -> Material_Flags;
};

}
