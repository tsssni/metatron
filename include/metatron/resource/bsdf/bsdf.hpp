#pragma once
#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/resource/bsdf/interface.hpp>

namespace mtt::bsdf {
    auto init() noexcept -> void;

    template<typename T>
    concept Scattering = requires(
        T const& t,
        fv3 const& wo, fv3 const& wi, f32 u0,
        math::Context const& ctx, fv3 const& u1
    ) {
        // u for lobe selection replay
        { t(wo, wi, u0) } noexcept -> std::same_as<Interaction>;
        { t.sample(ctx, u1) } noexcept -> std::same_as<Interaction>;
        { t.flags() } noexcept -> std::same_as<Flags>;
    };

    auto lambert(f32 reflectance) noexcept -> f32;
    auto lambert(fv4 const& reflectance) noexcept -> fv4;

    auto fresnel(f32 cos_theta_i, f32 eta, f32 k) noexcept -> f32;
    auto fresnel(f32 cos_theta_i, fv4 const& eta, fv4 const& k) noexcept -> fv4;

    auto lambda(fv3 const& wo, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto smith_mask(fv3 const& wo, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto smith_shadow(fv3 const& wo, fv3 const& wi, f32 alpha_u, f32 alpha_v) noexcept -> f32;

    auto trowbridge_reitz(fv3 const& wm, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto visible_trowbridge_reitz(fv3 const& wo, fv3 const& wm, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto torrance_sparrow(
        bool reflective, f32 pr, f32 pt,
        fv4 const& F, f32 D, f32 G,
        fv3 const& wo, fv3 const& wi, fv3 const& wm,
        fv4 const& eta, f32 alpha_u, f32 alpha_v
    ) noexcept -> Interaction;
}
