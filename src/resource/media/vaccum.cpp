#include <metatron/resource/media/vaccum.hpp>

namespace mtt::media {
    auto Vaccum_Medium::sample(
        cref<eval::Context> ctx, f32 t_max, f32 u
    ) const noexcept -> opt<Interaction> {
        auto transmittance = ctx.spec;
        transmittance = 1.f;

        return Interaction{
            ctx.r.o + t_max * ctx.r.d,
            {},
            t_max,
            transmittance,
            {}, {}, {}, {}, {},
        };
    }
}
