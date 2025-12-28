#include <metatron/resource/media/vaccum.hpp>

namespace mtt::media {
    Vaccum_Medium::Vaccum_Medium(cref<Descriptor>) noexcept {}

    auto Vaccum_Medium::sample(
        cref<math::Context> ctx, f32 t_max, f32 u
    ) const noexcept -> opt<Interaction> {
        return Interaction{
            ctx.r.o + t_max * ctx.r.d,
            {}, t_max, fv4{1.f},
            {}, {}, {}, {}, {},
        };
    }
}
