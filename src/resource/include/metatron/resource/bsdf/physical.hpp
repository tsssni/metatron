#pragma once
#include <metatron/resource/bsdf/bsdf.hpp>

namespace mtt::bsdf {
    struct Physical_Bsdf final {
        auto static init() noexcept -> void;

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
            math::Vector<f32, 3> const& wo,
            math::Vector<f32, 3> const& wi
        ) const noexcept -> std::optional<Interaction>;
        auto sample(
            eval::Context const& ctx,
            math::Vector<f32, 3> const& u
        ) const noexcept -> std::optional<Interaction>;
        auto configure(Attribute const& attr) noexcept -> void;
        auto flags() const noexcept -> Flags;
        auto degrade() noexcept -> bool;

    private:
        std::vector<f32> static fresnel_reflectance_table;

        f32 alpha_u;
        f32 alpha_v;
        spectra::Stochastic_Spectrum spectrum;
        spectra::Stochastic_Spectrum eta;
        spectra::Stochastic_Spectrum k;
        spectra::Stochastic_Spectrum reflectance;
        spectra::Stochastic_Spectrum fresnel_reflectance;

        bool lambertian;
        bool dieletric;
        bool conductive;
        bool plastic;
    };
}
