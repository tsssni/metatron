#include <metatron/resource/media/heterogeneous.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/resource/volume/uniform.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::media {
    Heterogeneous_Medium::Heterogeneous_Medium(cref<Descriptor> desc) noexcept:
    phase(desc.phase),
    sigma_a(desc.sigma_a),
    sigma_s(desc.sigma_s),
    sigma_e(desc.sigma_e),
    density(desc.density),
    density_scale(desc.density_scale) {
        auto sigmaj = volume::Uniform_Volume{{density->bounding_box(), desc.dimensions}};
        stl::scheduler::instance().sync_parallel(
            sigmaj.dimensions(),
            [&](cref<uzv3> xyz) mutable {
                auto ijk = iv3{xyz};
                auto voxel_bbox = sigmaj.bounding_box(ijk);
                auto maj = math::low<f32>;

                auto b_min = density->to_index(voxel_bbox.p_min);
                auto b_max = density->to_index(voxel_bbox.p_max);
                auto p_min = math::min(b_min, b_max);
                auto p_max = math::max(b_min, b_max);

                for (auto i = p_min[0]; i <= p_max[0]; ++i)
                    for (auto j = p_min[1]; j <= p_max[1]; ++j)
                        for (auto k = p_min[2]; k <= p_max[2]; ++k)
                            maj = math::max(maj, (*std::as_const(density).data())[{i, j, k}]);
                sigmaj[ijk] = maj;
            }
        );

        auto& vec = stl::vector<volume::Volume>::instance();
        majorant = vec.push_back<volume::Uniform_Volume>(std::move(sigmaj));
    }

    auto Heterogeneous_Medium::sample(
        cref<math::Context> ctx, f32 t_max, f32 u
    ) const noexcept -> opt<Interaction> {
        auto sigma_a = (ctx.lambda & this->sigma_a) * density_scale;
        auto sigma_s = (ctx.lambda & this->sigma_s) * density_scale;
        auto sigma_t = sigma_a + sigma_s;
        auto sigma_maj = sigma_t;
        auto transmittance = fv4{1.f};
        auto density_maj = 0.f;
        auto distr = math::Exponential_Distribution{0.f};

        auto r = ctx.r;
        auto cell = majorant->to_index(r.o); auto offset = iv3{0};
        auto direction = math::foreach([](f32 x, auto){return math::sign(x);}, r.d);

        auto t_cell = t_max;
        auto t_boundary = t_max;
        auto t_transmitted = 0.f;

        auto update_majorant = [&](f32 t_max) -> void {
            auto inside = majorant->inside(cell);
            auto next_inside = majorant->inside(cell + offset);
            if (inside && !next_inside) {
                t_cell = t_boundary;
                return;
            }

            auto bbox = majorant->bounding_box(cell + offset);
            auto [t_enter, t_next, i_enter, i_next] = math::hitvi(r, bbox).value_or(
                std::make_tuple(t_boundary, t_boundary, 0uz, 0uz)
            );
            t_cell = t_next;
            cell += offset;
            offset = direction * iv3{
                i_next == 0, i_next == 1, i_next == 2
            };

            density_maj = (*majorant.data())[cell];
            sigma_maj = density_maj * sigma_t;
            distr = math::Exponential_Distribution(sigma_maj[0]);
        };

        if (!majorant->inside(cell)) {
            auto [t_enter, t_exit] = math::hit(r, majorant->bounding_box()).value_or(
                fv2{0.f}
            );
            r.o = r.o + t_enter * r.d;
            cell = math::clamp<i32, 3>(
                majorant->to_index(r.o), iv3{0}, majorant->dimensions() - 1
            );
        }
        update_majorant(t_max);

        auto update_transmittance = [&](f32 t) -> void {
            t_transmitted += t;
            t_boundary -= t;
            r.o += t * r.d;
            transmittance *= math::exp(-sigma_maj * t);
        };

        while (true) {
            auto t_u = distr.sample(u);
            if (true
            && t_boundary <= t_cell
            && (density_maj < math::epsilon<f32> || t_u >= t_boundary)) {
                update_transmittance(t_boundary);
                return Interaction{
                    r.o,
                    phase.to_phase(),
                    t_max,
                    transmittance,
                    {}, {}, {}, {}, {},
                };
            } else if (true
            && t_boundary > t_cell
            && (density_maj < math::epsilon<f32> || t_u >= t_cell)) {
                update_transmittance(t_cell);
                update_majorant(t_boundary);
            } else {
                update_transmittance(t_u);
                auto spectra_pdf = sigma_maj * transmittance;
                auto density = (*std::as_const(this->density).data())(r.o);

                return Interaction{
                    r.o,
                    phase.to_phase(),
                    t_transmitted,
                    transmittance,
                    density * sigma_a,
                    density * sigma_s,
                    math::max(sigma_maj - density * sigma_t, fv4{0.f}),
                    sigma_maj,
                    density * (sigma_e ? (ctx.lambda & sigma_e) : fv4{0.f}),
                };
            }
        }
    }
}
