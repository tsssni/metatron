#pragma once
#include <metatron/resource/phase/interaction.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/core/math/eval.hpp>
#include <metatron/core/stl/protocol.hpp>

namespace mtt::phase {
    struct Phase_Function final: stl::variant<Phase_Function, Henyey_Greenstein_Phase_Function> {
        using variant::variant;

        auto operator()(cref<fv3> wo, cref<fv3> wi) const noexcept -> opt<Interaction> {
            return visit([&](auto* p) noexcept { return (*p)(wo, wi); });
        }
        auto sample(cref<math::Context> ctx, cref<fv2> u) const noexcept -> opt<Interaction> {
            return visit([&](auto* p) noexcept { return p->sample(ctx, u); });
        }
    };
}
