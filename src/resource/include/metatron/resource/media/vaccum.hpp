#pragma once
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/phase/phase-function.hpp>

namespace mtt::media {
    struct Vaccum_Medium final {
        auto sample(
            cref<math::Context> ctx, f32 t_max, f32 u
        ) const noexcept -> opt<Interaction>;
    };
}
