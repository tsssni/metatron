#pragma once
#include <metatron/volume/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/math/grid/grid.hpp>
#include <unordered_map>

namespace metatron::media {
	auto constexpr grid_size = 64uz;
	using Medium_Grid = math::Grid<f32, grid_size, grid_size, grid_size>;

	struct Grid_Medium final: Medium {
		struct Cache final {
			math::Ray r{
				{math::maxv<f32>},
				{0.f}
			};
			math::Bounding_Box bbox{};
			f32 t_max{-1.f};
			f32 density_maj{0.f};
			spectra::Stochastic_Spectrum sigma_maj{};
			math::Exponential_Distribution distr{0.f};
		};

		Grid_Medium(
			Medium_Grid const* grid,
			std::unique_ptr<spectra::Spectrum> sigma_a,
			std::unique_ptr<spectra::Spectrum> sigma_s,
			std::unique_ptr<spectra::Spectrum> L,
			std::unique_ptr<phase::Phase_Function> phase,
			f32 density_scale = 1.0f
		);

		~Grid_Medium();
		
		auto sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction>;
		
	private:
		std::unique_ptr<spectra::Spectrum> sigma_a;
		std::unique_ptr<spectra::Spectrum> sigma_s;
		std::unique_ptr<spectra::Spectrum> L;
		std::unique_ptr<phase::Phase_Function> phase;

		Medium_Grid const* grid;
		f32 density_scale;

		static thread_local std::unordered_map<Grid_Medium const*, Cache> thread_caches;
	};
}
