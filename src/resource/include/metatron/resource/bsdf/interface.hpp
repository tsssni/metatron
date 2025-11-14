#pragma once
#include <metatron/resource/bsdf/bsdf.hpp>

namespace mtt::bsdf {
    struct Interface_Bsdf final {
        Interface_Bsdf(
            cref<stsp> spectrum
        ) noexcept;

        auto operator()(
            cref<fv3> wo, cref<fv3> wi
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<eval::Context> ctx, cref<fv3> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;
        auto degrade() noexcept -> bool;

    private:
        stsp spectrum;
    };
}
