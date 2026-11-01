#include <metatron/render/light/environment.hpp>
#include <metatron/core/distribution/sphere.hpp>
#include <metatron/core/math/cmath.hpp>

namespace mtt::render {

auto Environment_Light::operator()(
    core::Ray const& r, fv4 const& lambda
) const noexcept -> Light_Interaction {
    auto [radius, theta, phi] = core::cartesian_to_spherical(r.d);
    auto u = 1.f - phi / (2.f * core::pi);
    auto v = theta / core::pi;
    auto t = env_map({u, v}, {}, lambda);
    auto J = 2.f * core::pow<2>(core::pi) * core::sin(theta);
    return {
        .L = t,
        .wi = r.d,
        .p = r.o + r.d * 65504.f,
        .t = 65504.f,
        .pdf = core::guarded_div(env_map.pdf({u, v}), J),
    };
}

auto Environment_Light::sample(
    core::Ray_Context const& ctx,
    fv2 const& u
) const noexcept -> Light_Interaction {
    auto uv = env_map.sample(ctx, u);
    auto phi = (1.f - uv[0]) * 2.f * core::pi;
    auto theta = uv[1] * core::pi;
    auto wi = core::unit_spherical_to_cartesian({theta, phi});
    return (*this)({ctx.r.o, wi}, ctx.lambda);
}

auto Environment_Light::flags() const noexcept -> Light_Flags {
    return Light_Flags::inf;
}

}
