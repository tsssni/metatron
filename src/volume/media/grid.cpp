#include <metatron/volume/media/grid.hpp>
#include <metatron/volume/phase/henyey-greenstein.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::media {
	std::unordered_map<Grid_Medium const*, Grid_Medium::Cache> thread_local Grid_Medium::thread_caches;

    Grid_Medium::Grid_Medium(
        Medium_Grid const* grid,
        std::unique_ptr<spectra::Spectrum> sigma_a,
        std::unique_ptr<spectra::Spectrum> sigma_s,
        std::unique_ptr<spectra::Spectrum> Le,
		std::unique_ptr<phase::Phase_Function> phase,
        f32 density_scale
    ):
	grid(grid),
	sigma_a(std::move(sigma_a)),
	sigma_s(std::move(sigma_s)),
	Le(std::move(Le)),
	phase(std::move(phase)),
	density_scale(density_scale) {
		auto& cache = thread_caches[this];
	}

	Grid_Medium::~Grid_Medium() {
		thread_caches.erase(this);
	}
    
    auto Grid_Medium::sample(
        eval::Context const& ctx, 
        f32 t_max, 
        f32 u
    ) const -> std::optional<Interaction> {
		auto& cache = thread_caches[this];
		
		auto sigma_a = (ctx.L & (*this->sigma_a)) * density_scale;
		auto sigma_s = (ctx.L & (*this->sigma_s)) * density_scale;
		auto sigma_t = sigma_a + sigma_s;

		auto update_majorant = [&](f32 t_max) -> void {
			cache.bbox = grid->bounding_box(cache.r.o);
			cache.t_max = math::hit(cache.r, cache.bbox).value_or(t_max);
			cache.density_maj = (*grid)[grid->to_index(cache.r.o)] * density_scale;
			cache.sigma_maj = cache.density_maj * sigma_t;
			cache.distr = math::Exponential_Distribution(cache.sigma_maj.value.front());
		};

		if (false 
		|| math::length(ctx.r.o - cache.r.o) > 0.001f
		|| math::length(ctx.r.d - cache.r.d) > 0.001f) {
			cache.r = ctx.r;
			update_majorant(t_max);
		}

		auto t_boundary = t_max;
		auto t_offset = 0.005f / math::length(cache.r.d);
		auto t_transmitted = 0.f;
		auto transmittance = ctx.L;
		transmittance.value = std::vector<f32>(transmittance.lambda.size(), 1.f);
		auto update_transmittance = [&](f32 t) -> void {
			t_transmitted += t;
			t_boundary -= t;
			cache.t_max -= t;
			cache.r.o += t * cache.r.d;
			for (auto i = 0uz; i < transmittance.lambda.size(); i++) {
				transmittance.value[i] *= std::exp(-cache.sigma_maj.value[i] * t);
			}
		};

		while (true) {
			auto t_u = cache.distr.sample(u);
			if (t_boundary <= cache.t_max && (cache.density_maj == 0.f || t_u >= t_boundary)) {
				update_transmittance(t_boundary + t_offset);
				return Interaction{
					cache.r.o,
					phase.get(),
					t_max,
					transmittance.value.front(),
					transmittance,
					transmittance,
					{}, {}, {}, {}, {}
				};
			} else if (t_boundary > cache.t_max && (cache.density_maj == 0.f || t_u >= cache.t_max)) {
				update_transmittance(cache.t_max + t_offset);
				update_majorant(t_boundary);
			} else {
				update_transmittance(t_u);
				auto spectra_pdf = cache.sigma_maj * transmittance;
				auto density = (*grid)(cache.r.o);

				return Interaction{
					cache.r.o,
					phase.get(),
					t_transmitted,
					spectra_pdf.value.front(),
					spectra_pdf,
					transmittance,
					density * sigma_a,
					density * sigma_s,
					cache.sigma_maj - density * sigma_t,
					cache.sigma_maj,
					density * (ctx.L & *Le),
				};
			}
		}
    }
}
