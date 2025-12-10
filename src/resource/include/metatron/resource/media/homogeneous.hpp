#include <metatron/resource/media/medium.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::media {
    struct Homogeneous_Medium final {
        Phase phase;
        tag<spectra::Spectrum> sigma_a = entity<spectra::Spectrum>("/spectrum/zero");
        tag<spectra::Spectrum> sigma_s = entity<spectra::Spectrum>("/spectrum/zero");
        tag<spectra::Spectrum> sigma_e = entity<spectra::Spectrum>("/spectrum/zero");

        auto sample(
            cref<math::Context> ctx, f32 t_max, f32 u
        ) const noexcept -> opt<Interaction>;
    };
}
