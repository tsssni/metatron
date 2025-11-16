#pragma once
#include <metatron/resource/phase/phase-function.hpp>

namespace mtt::phase {
    struct Henyey_Greenstein_Phase_Function final {
        f32 g;

        auto operator()(
            cref<fv3> wo, cref<fv3> wi
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<eval::Context> ctx, cref<fv2> u
        ) const noexcept -> opt<Interaction>;
    };
}
