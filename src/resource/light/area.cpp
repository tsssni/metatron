#include <metatron/resource/light/area.hpp>

namespace mtt::light {
    auto Area_Light::operator()(
        cref<math::Ray> r, cref<fv4> lambda
    ) const noexcept -> opt<Interaction> {
        return {};
    }

    auto Area_Light::sample(
        cref<eval::Context> ctx, cref<fv2> u
    ) const noexcept -> opt<Interaction> {
        MTT_OPT_OR_RETURN(s_intr, shape->sample(ctx, u, primitive), {});
        return Interaction{
            .L = fv4{0.f}, // delay fetching L in integrator material interaction
            .wi = math::normalize(s_intr.p - ctx.r.o),
            .p = s_intr.p,
            .t = s_intr.t,
            .pdf = s_intr.pdf,
        };
    }

    auto Area_Light::flags() const noexcept -> Flags {
        return Flags(0);
    }
}
