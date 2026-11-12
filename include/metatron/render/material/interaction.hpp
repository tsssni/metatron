#pragma once
#include <metatron/render/texture/texture.hpp>
#include <metatron/core/math/ray.hpp>
#include <metatron/core/math/transform.hpp>
#include <metatron/core/stl/concept.hpp>

namespace mtt::render {

struct Bsdf_Interaction final {
    fv4 f;
    fv4 eta;
    fv3 wi;
    f32 pdf;
    bool degraded = false;
};

enum Bsdf_Flags {
    reflective = 1 << 0,
    transmissive = 1 << 1,
    specular = 1 << 2,
    bsdf_interface = 1 << 3,
};

template<typename T>
concept Scattering = requires(
    T const& t,
    fv3 const& wo, fv3 const& wi, f32 u0,
    core::Ray_Context const& ctx, fv3 const& u1
) {
    // u for lobe selection replay
    { t(wo, wi, u0) } noexcept -> std::same_as<Bsdf_Interaction>;
    { t.sample(ctx, u1) } noexcept -> std::same_as<Bsdf_Interaction>;
    { t.flags() } noexcept -> std::same_as<Bsdf_Flags>;
};

template<Scattering Bsdf>
struct Material_Interaction final {
    Bsdf bsdf;
    fv4 emission;
    fv3 normal = {0.f};
    bool degraded = false;
};

enum Material_Flags {
    material_interface = 1 << 0,
    emissive = 1 << 1,
};

}
