#pragma once
#include <metatron/resource/light/light.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::light {
    struct Spot_Light final {
        struct Descriptor final {
            tag<spectra::Spectrum> L;
            f32 falloff_start_theta;
            f32 falloff_end_theta;
        };
        Spot_Light() noexcept = default;
        Spot_Light(cref<Descriptor> desc) noexcept;

        auto operator()(
            cref<math::Ray> r, cref<stsp> spec
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<eval::Context> ctx, cref<fv2> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;

    private:
        tag<spectra::Spectrum> L;
        f32 falloff_start_cos_theta;
        f32 falloff_end_cos_theta;
    };
}
