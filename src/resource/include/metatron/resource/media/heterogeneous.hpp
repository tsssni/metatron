#pragma once
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/volume/volume.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>

namespace mtt::media {
    struct Heterogeneous_Medium final {
        struct Descriptor final {
            Phase phase;
            tag<spectra::Spectrum> sigma_a = spectra::Spectrum::spectra["zero"];
            tag<spectra::Spectrum> sigma_s = spectra::Spectrum::spectra["zero"];
            tag<spectra::Spectrum> sigma_e = spectra::Spectrum::spectra["zero"];
            uzv3 dimensions;
            tag<volume::Volume> density;
            f32 density_scale;
        };
        Heterogeneous_Medium() noexcept = default;
        Heterogeneous_Medium(cref<Descriptor> desc) noexcept;

        auto sample(
            cref<eval::Context> ctx, f32 t_max, f32 u
        ) const noexcept -> opt<Interaction>;

    private:
        Phase phase;
        tag<spectra::Spectrum> sigma_a;
        tag<spectra::Spectrum> sigma_s;
        tag<spectra::Spectrum> sigma_e;
        tag<volume::Volume> majorant;
        tag<volume::Volume> density;
        f32 density_scale;
    };
}
