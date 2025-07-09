#pragma once
#include <metatron/resource/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/math/grid/grid.hpp>

namespace mtt::media {
	auto constexpr grid_size = 64uz;
	using Medium_Grid = math::Grid<f32, grid_size, grid_size, grid_size>;

	struct Grid_Medium final: Medium {
		Grid_Medium(
			Medium_Grid const* grid,
			phase::Phase_Function const* phase,
			view<spectra::Spectrum> sigma_a,
			view<spectra::Spectrum> sigma_s,
			view<spectra::Spectrum> emission,
			f32 density_scale = 1.0f
		);

		auto sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction>;
		
	private:
		phase::Phase_Function const* phase;
		view<spectra::Spectrum> sigma_a;
		view<spectra::Spectrum> sigma_s;
		view<spectra::Spectrum> emission;

		Medium_Grid const* grid;
		f32 density_scale;
	};
}
