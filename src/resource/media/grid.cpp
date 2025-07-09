#include <metatron/resource/media/grid.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>

namespace mtt::media {
    Grid_Medium::Grid_Medium(
        Medium_Grid const* grid,
		phase::Phase_Function const* phase,
		view<spectra::Spectrum> sigma_a,
		view<spectra::Spectrum> sigma_s,
		view<spectra::Spectrum> emission,
        f32 density_scale
    ):
	grid(grid),
	phase(phase),
	sigma_a(sigma_a),
	sigma_s(sigma_s),
	emission(emission),
	density_scale(density_scale) {}

    auto Grid_Medium::sample(
        eval::Context const& ctx, 
        f32 t_max, 
        f32 u
    ) const -> std::optional<Interaction> {
		auto sigma_a = (ctx.spec & this->sigma_a) * density_scale;
		auto sigma_s = (ctx.spec & this->sigma_s) * density_scale;
		auto sigma_t = sigma_a + sigma_s;

		auto r = ctx.r;
		auto bbox = math::Bounding_Box{};
		auto t_next = t_max;
		auto density_maj = 0.f;
		auto sigma_maj = sigma_t;
		auto distr = math::Exponential_Distribution{0.f};

		auto update_majorant = [&](f32 t_max) -> void {
			bbox = grid->bounding_box(r.o);
			t_next = math::hit(r, bbox).value_or(t_max);
			density_maj = (*grid)[grid->to_index(r.o)];
			sigma_maj = density_maj * sigma_t;
			distr = math::Exponential_Distribution(sigma_maj.value[0]);
		};
		update_majorant(t_max);

		auto t_boundary = t_max;
		auto t_offset = 0.005f / math::length(r.d);
		auto t_transmitted = 0.f;
		auto transmittance = ctx.spec;
		transmittance = 1.f;

		auto update_transmittance = [&](f32 t) -> void {
			t_transmitted += t;
			t_boundary -= t;
			t_next -= t;
			r.o += t * r.d;
			transmittance.value *= math::foreach([&](f32 value, usize i) {
				return std::exp(-value * t);
			}, sigma_maj.value);
		};

		while (true) {
			auto t_u = distr.sample(u);
			if (t_boundary <= t_next && (density_maj < math::epsilon<f32> || t_u >= t_boundary)) {
				update_transmittance(t_boundary + t_offset);
				return Interaction{
					r.o,
					phase->clone({ctx.spec}),
					t_max,
					transmittance.value[0],
					transmittance,
					transmittance,
					{}, {}, {}, {}, {}
				};
			} else if (t_boundary > t_next && (density_maj < math::epsilon<f32> || t_u >= t_next)) {
				update_transmittance(t_next + t_offset);
				update_majorant(t_boundary);
			} else {
				update_transmittance(t_u);
				auto spectra_pdf = sigma_maj * transmittance;
				auto density = (*grid)(r.o);

				return Interaction{
					r.o,
					phase->clone({ctx.spec}),
					t_transmitted,
					spectra_pdf.value[0],
					spectra_pdf,
					transmittance,
					density * sigma_a,
					density * sigma_s,
					sigma_maj - density * sigma_t,
					sigma_maj,
					density * (ctx.spec & emission)
				};
			}
		}
    }
}
