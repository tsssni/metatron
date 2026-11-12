#pragma once
#include <metatron/render/media/interaction.hpp>

namespace mtt::render {

struct [[mtt::reflection]] Vaccum_Phase_Function final {
    auto operator()(
        fv3 const& wo, fv3 const& wi
    ) const noexcept -> Phase_Interaction;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> Phase_Interaction;
};

struct [[mtt::reflection]] Vaccum_Medium final {
    struct Iterator final {
        core::Ray r;
        f32 t_max;
        auto march(f32 u) noexcept -> Medium_Interaction<Vaccum_Phase_Function>;
    };

    auto begin(
        core::Ray_Context const& ctx, f32 t_max
    ) const noexcept -> Iterator;
};

}
