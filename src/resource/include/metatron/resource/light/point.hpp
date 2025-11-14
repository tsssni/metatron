#pragma once
#include <metatron/resource/light/light.hpp>

namespace mtt::light {
    struct Point_Light final {
        tag<spectra::Spectrum> L;

        auto operator()(
            cref<math::Ray> r, cref<stsp> spec
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<eval::Context> ctx, cref<fv2> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
