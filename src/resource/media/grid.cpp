#include <metatron/resource/media/grid.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>

namespace metatron::media {
	std::unordered_map<Grid_Medium const*, Grid_Medium::Cache> thread_local Grid_Medium::thread_cache;

    Grid_Medium::Grid_Medium(
        Medium_Grid const* grid,
		phase::Phase_Function const* phase,
		spectra::Spectrum const* sigma_a,
		spectra::Spectrum const* sigma_s,
		spectra::Spectrum const* emission,
        f32 density_scale
    ):
	grid(grid),
	phase(phase),
	sigma_a(sigma_a),
	sigma_s(sigma_s),
	emission(emission),
	density_scale(density_scale) {}

	Grid_Medium::~Grid_Medium() {
		thread_cache.erase(this);
	}
    
    auto Grid_Medium::sample(
        eval::Context const& ctx, 
        f32 t_max, 
        f32 u
    ) const -> std::optional<Interaction> {
		auto& cache = thread_cache[this];
		
		auto sigma_a = (ctx.spec & (*this->sigma_a)) * density_scale;
		auto sigma_s = (ctx.spec & (*this->sigma_s)) * density_scale;
		auto sigma_t = sigma_a + sigma_s;

		auto update_majorant = [&](f32 t_max) -> void {
			cache.bbox = grid->bounding_box(cache.r.o);
			cache.t_max = math::hit(cache.r, cache.bbox).value_or(t_max);
			cache.density_maj = (*grid)[grid->to_index(cache.r.o)];
			cache.sigma_maj = cache.density_maj * sigma_t;
			cache.distr = math::Exponential_Distribution(cache.sigma_maj.value[0]);
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
		auto transmittance = ctx.spec;
		transmittance = 1.f;

		auto update_transmittance = [&](f32 t) -> void {
			t_transmitted += t;
			t_boundary -= t;
			cache.t_max -= t;
			cache.r.o += t * cache.r.d;
			transmittance.value *= math::foreach([&](f32 value, usize i) {
				return std::exp(-value * t);
			}, cache.sigma_maj.value);
		};

		while (true) {
			auto t_u = cache.distr.sample(u);
			if (t_boundary <= cache.t_max && (cache.density_maj < math::epsilon<f32> || t_u >= t_boundary)) {
				update_transmittance(t_boundary + t_offset);
				return Interaction{
					cache.r.o,
					phase->clone({ctx.spec}),
					t_max,
					transmittance.value[0],
					transmittance,
					transmittance,
					{}, {}, {}, {}, {}
				};
			} else if (t_boundary > cache.t_max && (cache.density_maj < math::epsilon<f32> || t_u >= cache.t_max)) {
				update_transmittance(cache.t_max + t_offset);
				update_majorant(t_boundary);
			} else {
				update_transmittance(t_u);
				auto spectra_pdf = cache.sigma_maj * transmittance;
				auto density = (*grid)(cache.r.o);

				return Interaction{
					cache.r.o,
					phase->clone({ctx.spec}),
					t_transmitted,
					spectra_pdf.value[0],
					spectra_pdf,
					transmittance,
					density * sigma_a,
					density * sigma_s,
					cache.sigma_maj - density * sigma_t,
					cache.sigma_maj,
					density * (ctx.spec & *emission)
				};
			}
		}
    }
}
