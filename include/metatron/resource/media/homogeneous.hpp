#pragma once
#include <metatron/resource/media/interaction.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::media {
    struct Homogeneous_Medium final {
        Phase phase;
        spectra::Spectrum sigma_a = spectra::Spectrum::entity("/spectrum/zero");
        spectra::Spectrum sigma_s = spectra::Spectrum::entity("/spectrum/zero");
        spectra::Spectrum sigma_e = spectra::Spectrum::entity("/spectrum/zero");

        struct Iterator final {
            Homogeneous_Medium const* medium;
            math::Ray r;
            fv4 lambda;
            f32 t_max;
            auto march(f32 u) noexcept -> opt<Interaction>;
        };

        auto begin(
            cref<math::Context> ctx, f32 t_max
        ) const noexcept -> Iterator;
    };
}
