#include <metatron/resource/light/parallel.hpp>

namespace mtt::light {
    auto Parallel_Light::operator()(
        cref<math::Ray> r, cref<fv4> lambda
    ) const noexcept -> opt<Interaction> {
        return {};
    }

    auto Parallel_Light::sample(
        cref<math::Context> ctx, cref<fv2> u
    ) const noexcept -> opt<Interaction> {
        auto constexpr wi = fv3{0.f, 0.f, -1.f};
        return Interaction{
            .L = ctx.lambda & L,
            .wi = wi,
            .p = ctx.r.o + 65504.f * wi,
            .t = 65504.f,
            .pdf = 1.f,
        };
    }

    auto Parallel_Light::flags() const noexcept -> Flags {
        return Flags(Flags::delta | Flags::inf);
    }
}
