#include <metatron/resource/light/point.hpp>

namespace mtt::light {
    auto Point_Light::operator()(
        cref<math::Ray> r, cref<fv4> lambda
    ) const noexcept -> opt<Interaction> {
        return {};
    }

    auto Point_Light::sample(
        cref<eval::Context> ctx, cref<fv2> u
    ) const noexcept -> opt<Interaction> {
        auto wi = math::normalize(-ctx.r.o);
        auto r = math::length(ctx.r.o);
        return Interaction{
            .L = (ctx.lambda & L) / (r * r),
            .wi = wi,
            .p = {0.f},
            .t = r,
            .pdf = 1.f,
        };
    }

    auto Point_Light::flags() const noexcept -> Flags {
        return delta;
    }
}
