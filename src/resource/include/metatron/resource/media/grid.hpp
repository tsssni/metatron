#pragma once
#include <metatron/resource/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/math/grid/grid.hpp>

namespace mtt::media {
	auto constexpr grid_size = 64uz;
	using Medium_Grid = math::Grid<f32, grid_size, grid_size, grid_size>;

	struct Grid_Medium final {
		Grid_Medium(
			view<Medium_Grid> grid,
			poly<phase::Phase_Function> phase,
			view<spectra::Spectrum> sigma_a,
			view<spectra::Spectrum> sigma_s,
			view<spectra::Spectrum> emission,
			f32 density_scale = 1.0f
		) noexcept;

		auto sample(
			eval::Context const& ctx, f32 t_max, f32 u
		) const noexcept -> std::optional<Interaction>;
		
	private:
		poly<phase::Phase_Function> phase;
		view<spectra::Spectrum> sigma_a;
		view<spectra::Spectrum> sigma_s;
		view<spectra::Spectrum> emission;

		view<Medium_Grid> grid;
		f32 density_scale;
	};
}
