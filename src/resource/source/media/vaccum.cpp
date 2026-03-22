#include <metatron/resource/media/vaccum.hpp>

namespace mtt::media {
    auto Vaccum_Medium::Iterator::march(f32 u) noexcept -> opt<Interaction> {
        return Interaction{
            {}, r.o + t_max * r.d,
            t_max, fv4{1.f},
            {}, {}, {}, {}, {},
        };
    }

    Vaccum_Medium::Vaccum_Medium(cref<Descriptor>) noexcept {}

    auto Vaccum_Medium::begin(
        cref<math::Context> ctx, f32 t_max
    ) const noexcept -> obj<media::Iterator> {
        return make_obj<media::Iterator, Iterator>(ctx.r, t_max);
    }
}
