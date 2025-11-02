#pragma once
#include <metatron/resource/media/medium.hpp>
#include <metatron/resource/volume/volume.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>

namespace mtt::media {
    struct Heterogeneous_Medium final {
        struct Descriptor final {
            Phase phase;
            stl::proxy<spectra::Spectrum> sigma_a;
            stl::proxy<spectra::Spectrum> sigma_s;
            stl::proxy<spectra::Spectrum> sigma_e;
            math::Vector<usize, 3> dimensions;
            stl::proxy<volume::Volume> density;
            f32 density_scale;
        };
        Heterogeneous_Medium(Descriptor const& desc) noexcept;

        auto sample(
            eval::Context const& ctx, f32 t_max, f32 u
        ) const noexcept -> std::optional<Interaction>;

    private:
        Phase phase;
        stl::proxy<spectra::Spectrum> sigma_a;
        stl::proxy<spectra::Spectrum> sigma_s;
        stl::proxy<spectra::Spectrum> sigma_e;
        stl::proxy<volume::Volume> sigma_majorant;
        stl::proxy<volume::Volume> const density;
        f32 density_scale;
    };
}
