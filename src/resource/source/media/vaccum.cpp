#include <metatron/resource/media/vaccum.hpp>

namespace mtt::media {
    auto Vaccum_Medium::Iterator::march(f32 u) noexcept -> Interaction<phase::Vaccum_Phase_Function> {
        return {
            {}, r.o + t_max * r.d,
            t_max, fv4{1.f},
            {}, {}, {}, {}, {},
        };
    }

    Vaccum_Medium::Vaccum_Medium(Descriptor const&) noexcept {}

    auto Vaccum_Medium::begin(
        math::Context const& ctx, f32 t_max
    ) const noexcept -> Iterator {
        return {ctx.r, t_max};
    }
}
