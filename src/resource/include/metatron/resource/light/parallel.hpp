#pragma once
#include <metatron/resource/light/light.hpp>

namespace mtt::light {
    struct Parallel_Light final {
        tag<spectra::Spectrum> L;

        auto operator()(
            cref<math::Ray> r, cref<fv4> lambda
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
