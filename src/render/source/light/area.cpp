#include <metatron/render/light/area.hpp>

namespace mtt::render {

auto Area_Light::operator()(
    core::Ray const& r, fv4 const& lambda
) const noexcept -> Light_Interaction {
    return {};
}

auto Area_Light::sample(
    core::Ray_Context const& ctx, fv2 const& u
) const noexcept -> Light_Interaction {
    auto s_intr = shape.sample(ctx, u, primitive);
    return {
        .L = fv4{0.f}, // delay fetching L in integrator material interaction
        .wi = core::normalize(s_intr.p - ctx.r.o),
        .p = s_intr.p,
        .t = s_intr.t,
        .pdf = s_intr.pdf,
    };
}

auto Area_Light::flags() const noexcept -> Light_Flags {
    return Light_Flags(0);
}

}
