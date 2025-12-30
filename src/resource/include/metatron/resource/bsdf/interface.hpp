#pragma once
#include <metatron/resource/bsdf/bsdf.hpp>

namespace mtt::bsdf {
    struct Interface_Bsdf final {
        struct Descriptor final {};
        Interface_Bsdf(cref<Descriptor>) noexcept;
        Interface_Bsdf() noexcept = default;

        auto operator()(
            cref<fv3> wo, cref<fv3> wi
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<math::Context> ctx, cref<fv3> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;

    private:
        u32 padding;
    };
}
