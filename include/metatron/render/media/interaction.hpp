#pragma once
#include <metatron/render/spectra/spectrum.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>
#include <metatron/core/stl/concept.hpp>

namespace mtt::render {

struct Phase_Interaction final {
    fv4 f;
    fv3 wi;
    f32 pdf;
};

template<typename T>
concept Phase_Function = requires(
    T const& t,
    fv3 const& wo, fv3 const& wi,
    core::Ray_Context const& ctx, fv2 const& u
) {
    { t(wo, wi) } noexcept -> std::same_as<Phase_Interaction>;
    { t.sample(ctx, u) } noexcept -> std::same_as<Phase_Interaction>;
};

template<render::Phase_Function Phase_Function>
struct Medium_Interaction final {
    Phase_Function phase;
    fv3 p;
    f32 t;
    fv4 transmittance;
    fv4 sigma_a;
    fv4 sigma_s;
    fv4 sigma_n;
    fv4 sigma_maj;
    fv4 sigma_e;
};

}
