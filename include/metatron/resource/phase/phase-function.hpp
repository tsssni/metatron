#pragma once
#include <metatron/resource/phase/henyey-greenstein.hpp>

namespace mtt::phase {
    struct Phase_Function final: stl::variant<Phase_Function, Henyey_Greenstein_Phase_Function> {
        using variant::variant;

        auto operator()(fv3 const& wo, fv3 const& wi) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return (*p)(wo, wi); });
        }
        auto sample(math::Context const& ctx, fv2 const& u) const noexcept -> Interaction {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
    };
}
