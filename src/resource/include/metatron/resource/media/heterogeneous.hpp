#pragma once
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/volume/volume.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>

namespace mtt::media {
    struct Heterogeneous_Medium final {
        struct Descriptor final {
            Phase phase;
            proxy<spectra::Spectrum> sigma_a = spectra::Spectrum::spectra["zero"];
            proxy<spectra::Spectrum> sigma_s = spectra::Spectrum::spectra["zero"];
            proxy<spectra::Spectrum> sigma_e = spectra::Spectrum::spectra["zero"];
            math::Vector<usize, 3> dimensions;
            proxy<volume::Volume> density;
            f32 density_scale;
        };
        Heterogeneous_Medium() noexcept = default;
        Heterogeneous_Medium(Descriptor const& desc) noexcept;

        auto sample(
            eval::Context const& ctx, f32 t_max, f32 u
        ) const noexcept -> std::optional<Interaction>;

    private:
        Phase phase;
        proxy<spectra::Spectrum> sigma_a;
        proxy<spectra::Spectrum> sigma_s;
        proxy<spectra::Spectrum> sigma_e;
        proxy<volume::Volume> majorant;
        proxy<volume::Volume> density;
        f32 density_scale;
    };
}
