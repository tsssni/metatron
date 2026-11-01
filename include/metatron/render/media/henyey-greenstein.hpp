#pragma once
#include <metatron/render/media/interaction.hpp>

namespace mtt::render {

struct Henyey_Greenstein_Phase_Function final {
    f32 g;

    auto operator()(
        fv3 const& wo, fv3 const& wi
    ) const noexcept -> Phase_Interaction;
    auto sample(
        core::Ray_Context const& ctx, fv2 const& u
    ) const noexcept -> Phase_Interaction;
};

}
