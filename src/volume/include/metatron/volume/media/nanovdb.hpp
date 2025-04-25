#pragma once
#include <metatron/volume/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/math/grid/grid.hpp>
#include <metatron/core/spectra/constant.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <nanovdb/NanoVDB.h>
#include <nanovdb/util/IO.h>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace metatron::media {
    struct Nanovdb_Medium final: Medium {
		struct Cache final {
			math::Ray r{
				{math::maxv<f32>},
				{0.f}
			};
			math::Bounding_Box bbox{};
			f32 t_max{-1.f};
			f32 density_inactive{0.f};
			f32 density_maj{0.f};
			spectra::Stochastic_Spectrum sigma_maj{};
			math::Exponential_Distribution distr{0.f};
		};

        Nanovdb_Medium(
            std::string_view path,
            std::unique_ptr<spectra::Spectrum> sigma_a,
            std::unique_ptr<spectra::Spectrum> sigma_s,
            std::unique_ptr<spectra::Spectrum> Le,
			std::unique_ptr<phase::Phase_Function> phase,
            f32 density_scale = 1.0f
        );

		~Nanovdb_Medium();
        
        auto sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction>;
        
    private:
        std::unique_ptr<spectra::Spectrum> sigma_a;
        std::unique_ptr<spectra::Spectrum> sigma_s;
        std::unique_ptr<spectra::Spectrum> Le;
		std::unique_ptr<phase::Phase_Function> phase;

		std::unique_ptr<math::Grid<f32, 64, 64, 64>> grid;
        f32 density_scale;
        
        nanovdb::GridHandle<> handle;
        nanovdb::FloatGrid const* nanovdb_grid;

		static thread_local std::unordered_map<Nanovdb_Medium const*, Cache> thread_caches;
    };
}
