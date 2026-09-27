#pragma once
#include <metatron/resource/phase/interaction.hpp>
#include <metatron/core/math/eval.hpp>

namespace mtt::phase {
    struct Henyey_Greenstein_Phase_Function final {
        f32 g;

        auto operator()(
            fv3 const& wo, fv3 const& wi
        ) const noexcept -> Interaction;
        auto sample(
            math::Context const& ctx, fv2 const& u
        ) const noexcept -> Interaction;
    };
}
