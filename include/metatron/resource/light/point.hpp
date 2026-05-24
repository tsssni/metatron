#pragma once
#include <metatron/resource/light/interaction.hpp>

namespace mtt::light {
    struct Point_Light final {
        spectra::Spectrum L;

        auto operator()(
            cref<math::Ray> r, cref<fv4> lambda
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
    };
}
