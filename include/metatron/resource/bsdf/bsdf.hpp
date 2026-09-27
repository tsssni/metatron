#pragma once
#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/resource/bsdf/interface.hpp>

namespace mtt::bsdf {
    auto init() noexcept -> void;

    struct Bsdf final: stl::variant<Bsdf, Physical_Bsdf, Interface_Bsdf> {
        using variant::variant;

        // u for lobe selection replay
        auto operator()(fv3 const& wo, fv3 const& wi, f32 u = -1) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return (*p)(wo, wi, u); });
        }
        auto sample(math::Context const& ctx, fv3 const& u) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto flags() const noexcept -> Flags {
            return visit([](auto* p) noexcept { return p->flags(); });
        }
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
