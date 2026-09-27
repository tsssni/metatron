#pragma once
#include <metatron/resource/phase/interaction.hpp>
#include <metatron/core/math/eval.hpp>

namespace mtt::phase {
    struct Vaccum_Phase_Function final {
        u32 padding = 0u;

        auto operator()(
            fv3 const& wo, fv3 const& wi
        ) const noexcept -> Interaction;
        auto sample(
            math::Context const& ctx, fv2 const& u
        ) const noexcept -> Interaction;
    };
}
