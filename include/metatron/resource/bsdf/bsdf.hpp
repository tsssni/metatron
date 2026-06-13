#pragma once
#include <metatron/resource/bsdf/physical.hpp>
#include <metatron/resource/bsdf/interface.hpp>

namespace mtt::bsdf {
    struct Bsdf final: stl::variant<Bsdf, Physical_Bsdf, Interface_Bsdf> {
        using variant::variant;

        auto static init() noexcept -> void;

        auto operator()(cref<fv3> wo, cref<fv3> wi) const noexcept -> opt<Interaction> {
            return visit([&](auto* p) noexcept { return (*p)(wo, wi); });
        }
        auto sample(cref<math::Context> ctx, cref<fv3> u) const noexcept -> opt<Interaction> {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
        auto flags() const noexcept -> Flags {
            return visit([](auto* p) noexcept { return p->flags(); });
        }
    };

    auto lambert(f32 reflectance) noexcept -> f32;
    auto lambert(cref<fv4> reflectance) noexcept -> fv4;

    auto fresnel(f32 cos_theta_i, f32 eta, f32 k) noexcept -> f32;
    auto fresnel(f32 cos_theta_i, cref<fv4> eta, cref<fv4> k) noexcept -> fv4;

    auto lambda(cref<fv3> wo, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto smith_mask(cref<fv3> wo, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto smith_shadow(cref<fv3> wo, cref<fv3> wi, f32 alpha_u, f32 alpha_v) noexcept -> f32;

    auto trowbridge_reitz(cref<fv3> wm, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto visible_trowbridge_reitz(cref<fv3> wo, cref<fv3> wm, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto torrance_sparrow(
        bool reflective, f32 pr, f32 pt,
        cref<fv4> F, f32 D, f32 G,
        cref<fv3> wo, cref<fv3> wi, cref<fv3> wm,
        cref<fv4> eta, f32 alpha_u, f32 alpha_v
    ) noexcept -> opt<Interaction>;
}
