#pragma once
#include <metatron/resource/spectra/stochastic.hpp>
#include <metatron/resource/eval/context.hpp>
#include <metatron/core/math/vector.hpp>

namespace mtt::bsdf {
    struct Interaction final {
        stsp f;
        fv3 wi;
        f32 pdf;
        bool degraded{false};
    };

    MTT_POLY_METHOD(bsdf_sample, sample);
    MTT_POLY_METHOD(bsdf_flags, flags);
    MTT_POLY_METHOD(bsdf_degrade, degrade);

    enum Flags {
        reflective = 1 << 0,
        transmissive = 1 << 1,
        interface = 1 << 2,
    };

    struct Bsdf final: pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, auto (
        cref<fv3> wo, cref<fv3> wi
    ) const noexcept -> opt<Interaction>>
    ::add_convention<bsdf_sample, auto (
        cref<eval::Context> ctx, cref<fv3> u
    ) const noexcept -> opt<Interaction>>
    ::add_convention<bsdf_flags, auto () const noexcept -> Flags>
    ::add_convention<bsdf_degrade, auto () noexcept -> bool>
    ::add_skill<pro::skills::as_view>
    ::build {};

    auto lambert(f32 reflectance) -> f32;
    auto lambert(cref<stsp> reflectance) -> stsp;

    auto fresnel(f32 cos_theta_i, f32 eta, f32 k) -> f32;
    auto fresnel(f32 cos_theta_i, cref<stsp> eta, cref<stsp> k) noexcept -> stsp;

    auto lambda(cref<fv3> wo, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto smith_mask(cref<fv3> wo, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto smith_shadow(cref<fv3> wo, cref<fv3> wi, f32 alpha_u, f32 alpha_v) noexcept -> f32;

    auto trowbridge_reitz(cref<fv3> wm, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto visible_trowbridge_reitz(cref<fv3> wo, cref<fv3> wm, f32 alpha_u, f32 alpha_v) noexcept -> f32;
    auto torrance_sparrow(
        bool reflective, f32 pr, f32 pt,
        cref<stsp> F, f32 D, f32 G,
        cref<fv3> wo, cref<fv3> wi, cref<fv3> wm,
        cref<stsp> eta, f32 alpha_u, f32 alpha_v
    ) noexcept -> opt<Interaction>;
}
