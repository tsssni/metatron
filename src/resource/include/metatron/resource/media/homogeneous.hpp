#include <metatron/resource/media/medium.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::media {
    struct Homogeneous_Medium final {
        Phase phase;
        tag<spectra::Spectrum> sigma_a = entity<spectra::Spectrum>("/spectrum/zero");
        tag<spectra::Spectrum> sigma_s = entity<spectra::Spectrum>("/spectrum/zero");
        tag<spectra::Spectrum> sigma_e = entity<spectra::Spectrum>("/spectrum/zero");

        struct Iterator final {
            Homogeneous_Medium const* medium;
            math::Ray r;
            fv4 lambda;
            f32 t_max;
            auto march(f32 u) noexcept -> opt<Interaction>;
        };

        auto begin(
            cref<math::Context> ctx, f32 t_max
        ) const noexcept -> obj<media::Iterator>;
    };
}
