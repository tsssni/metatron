#pragma once
#include <metatron/volume/media/medium.hpp>
#include <metatron/core/math/bounding-box.hpp>
#include <metatron/core/math/distribution/exponential.hpp>
#include <metatron/core/spectra/constant.hpp>
#include <metatron/core/spectra/stochastic.hpp>
#include <nanovdb/NanoVDB.h>
#include <nanovdb/util/IO.h>
#include <memory>
#include <string_view>

namespace metatron::media {
    struct Nanovdb_Medium final: Medium {
        Nanovdb_Medium(
            std::string_view path,
            std::unique_ptr<spectra::Spectrum> sigma_a,
            std::unique_ptr<spectra::Spectrum> sigma_s,
            std::unique_ptr<spectra::Spectrum> Le,
			std::unique_ptr<phase::Phase_Function> phase,
            f32 density_scale = 1.0f
        );
        
        auto sample(eval::Context const& ctx, f32 t_max, f32 u) const -> std::optional<Interaction>;
        
    private:
        std::unique_ptr<spectra::Spectrum> sigma_a;
        std::unique_ptr<spectra::Spectrum> sigma_s;
        std::unique_ptr<spectra::Spectrum> Le;
		std::unique_ptr<phase::Phase_Function> phase;
        f32 density_scale;
        
        nanovdb::GridHandle<> handle;
        nanovdb::FloatGrid const* grid;

		struct {
			math::Ray r{
				{math::maxv<f32>},
				{0.f}
			};
			math::Bounding_Box bbox{};
			f32 t_max{-1.f};
			f32 density_maj{0.f};
			spectra::Stochastic_Spectrum sigma_maj{};
			math::Exponential_Distribution distr{0.f};
		} mutable cache;
    };
}
