#include <metatron/render/media/henyey-greenstein.hpp>
#include <metatron/core/math/sphere.hpp>

namespace mtt::render {

auto Henyey_Greenstein_Phase_Function::operator()(
    fv3 const& wo, fv3 const& wi
) const noexcept -> Phase_Interaction {
    auto f = core::guarded_div(
        (1.f - g * g) / (4.f * core::pi),
        core::pow<3, 2>(1.f + g * g + 2.f * g * core::dot(-wo, wi))
    );
    return {fv4{f}, wi, f};
}

auto Henyey_Greenstein_Phase_Function::sample(
    core::Ray_Context const& ctx, fv2 const& u
) const noexcept -> Phase_Interaction {
    auto cos_theta = core::abs(g) < core::epsilon<f32> ? 1.f - 2.f * u[0]
                                                       : -1.f / (2.f * g) * (1.f + g * g - core::pow<2>(core::guarded_div(1.f - g * g, 1.f + g - 2.f * g * u[0])));
    auto phi = 2.f * core::pi * u[1];

    auto wi = core::unit_spherical_to_cartesian(cos_theta, phi);
    auto x = (*this)({0.f, -1.f, 0.f}, wi);
    return x;
}

}
