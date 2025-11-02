#include <metatron/resource/media/heterogeneous.hpp>
#include <metatron/resource/phase/henyey-greenstein.hpp>
#include <metatron/resource/volume/uniform.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/stl/optional.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::media {
    Heterogeneous_Medium::Heterogeneous_Medium(Descriptor const& desc) noexcept:
    phase(desc.phase),
    sigma_a(desc.sigma_a),
    sigma_s(desc.sigma_s),
    sigma_e(desc.sigma_e),
    density(desc.density),
    density_scale(desc.density_scale) {
        auto& vec = stl::poly_vector<volume::Volume>::instance();
        sigma_majorant = vec.emplace_back<volume::Uniform_Volume>(
            density->bounding_box(),
            desc.dimensions
        );
        stl::scheduler::instance().sync_parallel(
            sigma_majorant->dimensions(),
            [&](math::Vector<usize, 3> const& xyz) {
                auto ijk = math::Vector<i32, 3>{xyz};
                auto voxel_bbox = sigma_majorant->bounding_box(ijk);
                auto maj = math::low<f32>;

                std::swap(voxel_bbox.p_min[2], voxel_bbox.p_max[2]);
                auto p_min = density->to_index(voxel_bbox.p_min);
                auto p_max = density->to_index(voxel_bbox.p_max);
                p_min[2] *= -1;
                p_max[2] *= -1;

                for (auto i = p_min[0]; i <= p_max[0]; ++i)
                    for (auto j = p_min[1]; j <= p_max[1]; ++j)
                        for (auto k = p_min[2]; k <= p_max[2]; ++k)
                            maj = std::max(maj, density->at({i, j, k}));
                (*sigma_majorant.data())[ijk] = maj;
            }
        );
    }

    auto Heterogeneous_Medium::sample(
        eval::Context const& ctx, 
        f32 t_max, 
        f32 u
    ) const noexcept -> std::optional<Interaction> {
        auto sigma_a = (ctx.spec & this->sigma_a.data()) * density_scale;
        auto sigma_s = (ctx.spec & this->sigma_s.data()) * density_scale;
        auto sigma_t = sigma_a + sigma_s;
        auto sigma_maj = sigma_t;
        auto transmittance = ctx.spec & spectra::Spectrum::spectra["one"].data();
        auto density_maj = 0.f;
        auto distr = math::Exponential_Distribution{0.f};

        auto r = ctx.r;
        auto cell = this->sigma_majorant->to_index(r.o); auto offset = math::Vector<i32, 3>{0};
        auto direction = math::foreach([](f32 x, auto){return math::sign(x);}, r.d);

        auto t_cell = t_max;
        auto t_boundary = t_max;
        auto t_transmitted = 0.f;

        auto update_majorant = [&](f32 t_max) -> void {
            auto inside = sigma_majorant->inside(cell);
            auto next_inside = sigma_majorant->inside(cell + offset);
            if (inside && !next_inside) {
                t_cell = t_boundary;
                return;
            }

            auto bbox = sigma_majorant->bounding_box(cell + offset);
            auto [t_enter, t_next, i_enter, i_next] = math::hitvi(r, bbox).value_or(
                std::make_tuple(t_boundary, t_boundary, 0uz, 0uz)
            );
            t_cell = t_next;
            cell += offset;
            offset = direction * math::Vector<i32, 3>{
                i_next == 0, i_next == 1, i_next == 2
            };

            density_maj = (*sigma_majorant.data())[cell];
            sigma_maj = density_maj * sigma_t;
            distr = math::Exponential_Distribution(sigma_maj.value[0]);
        };

        if (!sigma_majorant->inside(cell)) {
            auto [t_enter, t_exit] = math::hit(r, sigma_majorant->bounding_box()).value_or(
                math::Vector<f32, 2>{0.f}
            );
            r.o = r.o + t_enter * r.d;
            cell = math::clamp<i32, 3>(
                sigma_majorant->to_index(r.o), math::Vector<i32, 3>{0}, sigma_majorant->dimensions() - 1
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
                return Interaction{
                    r.o,
                    phase.to_phase(ctx.spec),
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
                auto density = (*this->density.data())(r.o);

                return Interaction{
                    r.o,
                    phase.to_phase(ctx.spec),
                    t_transmitted,
                    transmittance,
                    density * sigma_a,
                    density * sigma_s,
                    spectra::max(sigma_maj - density * sigma_t, {0.f}),
                    sigma_maj,
                    density * (sigma_e
                    ? ctx.spec & sigma_e.data()
                    : ctx.spec & spectra::Spectrum::spectra["zero"].data())
                };
            }
        }
    }
}
