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
        Spot_Light(cref<Descriptor> desc) noexcept;
        Spot_Light() noexcept = default;

        auto operator()(
            cref<math::Ray> r, cref<fv4> lambda
        ) const noexcept -> opt<Interaction>;
        auto sample(
            cref<math::Context> ctx, cref<fv2> u
        ) const noexcept -> opt<Interaction>;
        auto flags() const noexcept -> Flags;

    private:
        tag<spectra::Spectrum> L;
        f32 falloff_start_cos_theta;
        f32 falloff_end_cos_theta;
    };
}
