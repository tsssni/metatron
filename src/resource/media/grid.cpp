#include <metatron/resource/media/grid.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>

namespace mtt::media {
    Grid_Medium::Grid_Medium(
        view<math::Grid<f32>> grid,
        poly<phase::Phase_Function> phase,
        view<spectra::Spectrum> sigma_a,
        view<spectra::Spectrum> sigma_s,
        view<spectra::Spectrum> sigma_e,
        f32 density_scale
    ) noexcept:
    grid(grid),
    phase(phase),
    sigma_a(sigma_a),
    sigma_s(sigma_s),
    sigma_e(sigma_e),
    density_scale(density_scale) {}

    auto Grid_Medium::sample(
        eval::Context const& ctx, 
        f32 t_max, 
        f32 u
    ) const noexcept -> std::optional<Interaction> {
        auto sigma_a = (ctx.spec & this->sigma_a) * density_scale;
        auto sigma_s = (ctx.spec & this->sigma_s) * density_scale;
        auto sigma_t = sigma_a + sigma_s;
        auto sigma_maj = sigma_t;
        auto transmittance = ctx.spec & spectra::Spectrum::spectra["one"];
        auto density_maj = 0.f;
        auto distr = math::Exponential_Distribution{0.f};

        auto r = ctx.r;
        auto cell = grid->to_index(r.o);
        auto offset = math::Vector<i32, 3>{0};
        auto direction = math::foreach([](f32 x, auto){return math::sign(x);}, r.d);

        auto t_cell = t_max;
        auto t_boundary = t_max;
        auto t_transmitted = 0.f;

        auto update_majorant = [&](f32 t_max) -> void {
            auto inside = grid->inside(cell);
            auto next_inside = grid->inside(cell + offset);
            if (inside && !next_inside) {
                t_cell = t_boundary;
                return;
            }

            auto bbox = grid->bounding_box(cell + offset);
            auto [t_enter, t_next, i_enter, i_next] = math::hitvi(r, bbox).value_or(
                std::make_tuple(t_boundary, t_boundary, 0uz, 0uz)
            );
            t_cell = t_next;
            cell += offset;
            offset = direction * math::Vector<i32, 3>{
                i_next == 0, i_next == 1, i_next == 2
            };

            density_maj = (*grid)[cell];
            sigma_maj = density_maj * sigma_t;
            distr = math::Exponential_Distribution(sigma_maj.value[0]);
        };

        if (!grid->inside(cell)) {
            auto [t_enter, t_exit] = math::hit(r, grid->bounding_box()).value_or(
                math::Vector<f32, 2>{0.f}
            );
            r.o = r.o + t_enter * r.d;
            cell = math::clamp<i32, 3>(
                grid->to_index(r.o), math::Vector<i32, 3>{0}, grid->dimensions() - 1
            );
        }
        update_majorant(t_max);

        auto update_transmittance = [&](f32 t) -> void {
            t_transmitted += t;
            t_boundary -= t;
            r.o += t * r.d;
            transmittance.value *= math::foreach([&](f32 value, usize i) {
                return std::exp(-value * t);
            }, sigma_maj.value);
        };

        while (true) {
            auto t_u = distr.sample(u);
            if (true
            && t_boundary <= t_cell
            && (density_maj < math::epsilon<f32> || t_u >= t_boundary)) {
                update_transmittance(t_boundary);
                auto phase = this->phase;
                phase->configure({ctx.spec});
                return Interaction{
                    r.o,
                    phase,
                    t_max,
                    transmittance.value[0],
                    transmittance,
                    transmittance,
                    {}, {}, {}, {}, {}
                };
            } else if (true
            && t_boundary > t_cell
            && (density_maj < math::epsilon<f32> || t_u >= t_cell)) {
                update_transmittance(t_cell);
                update_majorant(t_boundary);
            } else {
                update_transmittance(t_u);
                auto spectra_pdf = sigma_maj * transmittance;
                auto density = (*grid)(r.o);

                auto phase = this->phase;
                phase->configure({ctx.spec});
                return Interaction{
                    r.o,
                    phase,
                    t_transmitted,
                    spectra_pdf.value[0],
                    spectra_pdf,
                    transmittance,
                    density * sigma_a,
                    density * sigma_s,
                    spectra::max(sigma_maj - density * sigma_t, {0.f}),
                    sigma_maj,
                    density * (ctx.spec & sigma_e)
                };
            }
        }
    }
}
