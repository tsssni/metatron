#include <metatron/render/media/vaccum.hpp>

namespace mtt::render {

auto Vaccum_Phase_Function::operator()(
    fv3 const& wo, fv3 const& wi
) const noexcept -> Phase_Interaction { return {}; }

auto Vaccum_Phase_Function::sample(
    core::Ray_Context const& ctx, fv2 const& u
) const noexcept -> Phase_Interaction { return {}; }

auto Vaccum_Medium::Iterator::march(f32 u) noexcept -> Medium_Interaction<Vaccum_Phase_Function> {
    return {
        {},
        r.o + t_max * r.d,
        t_max,
        fv4{1.f},
        {},
        {},
        {},
        {},
        {},
    };
}

auto Vaccum_Medium::begin(
    core::Ray_Context const& ctx, f32 t_max
) const noexcept -> Iterator {
    return {ctx.r, t_max};
}

}
