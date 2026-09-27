#pragma once
#include <metatron/resource/bsdf/interaction.hpp>
#include <metatron/core/stl/stack.hpp>

namespace mtt::bsdf {
    namespace physical {
        auto init() noexcept -> void;
    }

    struct Physical_Bsdf final {
        Physical_Bsdf(
            fv4 const& reflectance,
            fv4 const& eta,
            fv4 const& k,
            f32 alpha_u,
            f32 alpha_v
        ) noexcept;

        buf<f32> static fresnel_reflectance_table;

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
        ) const noexcept -> Interaction;
        auto sample(
            math::Context const& ctx, fv3 const& u
        ) const noexcept -> Interaction;
        auto flags() const noexcept -> Flags;
        auto degrade() noexcept -> bool;

    private:
        fv4 reflectance;
        fv4 fresnel_reflectance;
        fv4 eta;
        fv4 k;
        f32 alpha_u;
        f32 alpha_v;

        bool lambertian;
        bool dieletric;
        bool conductive;
        bool plastic;
    };
}
