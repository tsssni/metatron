#pragma once
#include <metatron/resource/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/math/grid/grid.hpp>

namespace mtt::media {
    struct Grid_Medium final {
        Phase phase;
        stl::proxy<spectra::Spectrum> sigma_a;
        stl::proxy<spectra::Spectrum> sigma_s;
        stl::proxy<spectra::Spectrum> sigma_e;
        stl::proxy<math::Grid<f32>> grid;
        f32 density_scale;

        auto sample(
            eval::Context const& ctx, f32 t_max, f32 u
        ) const noexcept -> std::optional<Interaction>;
    };
}
