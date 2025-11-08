#include <metatron/resource/media/medium.hpp>
#include <metatron/core/stl/vector.hpp>

namespace mtt::media {
    struct Homogeneous_Medium final {
        Phase phase;
        proxy<spectra::Spectrum> sigma_a;
        proxy<spectra::Spectrum> sigma_s;
        proxy<spectra::Spectrum> sigma_e;

        auto sample(
            eval::Context const& ctx, f32 t_max, f32 u
        ) const noexcept -> std::optional<Interaction>;
    };
}
