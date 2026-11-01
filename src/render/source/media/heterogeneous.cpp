#include <metatron/render/media/heterogeneous.hpp>

namespace mtt::render {

namespace {

struct Context final {
    render::Volume density;
    core::Bounding_Box bbox;
    frw3v storage;
};

[[mtt::kernel]] auto major(Context& ctx, uv3 px, uv3 group, uv3 groups, u32 wavesize) noexcept -> void {
    auto voxel_size = (ctx.bbox.p_max - ctx.bbox.p_min) / fv3{groups};

    auto p0 = ctx.bbox.p_min + fv3{group} * voxel_size;
    auto p1 = ctx.bbox.p_min + (fv3{group} + 1.f) * voxel_size;
    auto b_min = ctx.density.to_index(p0);
    auto b_max = ctx.density.to_index(p1);
    auto p_min = core::min(b_min, b_max);
    auto p_max = core::max(b_min, b_max);

    auto maj = core::low<f32>;
    auto extent = uzv3{p_max - p_min + 1};
    auto count = core::prod(extent);
    auto lane = px[0] % wavesize;

    for (auto i = (usize)lane; i < count; i += wavesize) {
        auto idx = iv3{
            i / (extent[1] * extent[2]),
            (i / extent[2]) % extent[1],
            i % extent[2],
        };
        auto ijk = p_min + idx;
        maj = core::max(maj, ctx.density[ijk]);
    }
    maj = core::wave::max(maj);
    if (core::wave::elect()) ctx.storage[uzv3{group}] = fv4{maj, 0.f, 0.f, 0.f};
}

}

Heterogeneous_Medium::Iterator::Iterator(
    Heterogeneous_Medium const& m,
    core::Ray_Context const& ctx,
    f32 t
) noexcept {
    medium = &m;
    sigma_a = (ctx.lambda & medium->sigma_a) * medium->density_scale;
    sigma_s = (ctx.lambda & medium->sigma_s) * medium->density_scale;
    sigma_e = medium->sigma_e ? (ctx.lambda & medium->sigma_e) * medium->density_scale : fv4{0.f};
    sigma_maj = sigma_a + sigma_s;

    transmittance = fv4{1.f};
    r = ctx.r;
    cell = medium->majorant.to_index(r.o);
    offset = iv3{0};

    t_max = t;
    t_cell = t_max;
    t_boundary = t_max;
    t_transmitted = 0.f;

    if (!medium->majorant.inside(cell)) {
        auto [t_enter, t_exit] = core::hit(r, 1.f / r.d, medium->majorant.bounding_box());
        if (t_exit < -core::epsilon<f32> || t_enter > t_exit + core::epsilon<f32>) t_enter = 0.f;
        r.o = r.o + t_enter * r.d;
        cell = core::clamp<i32, 3>(medium->majorant.to_index(r.o), iv3{0}, medium->majorant.dimensions() - 1);
    }
    update_majorant(t_max);
}

auto Heterogeneous_Medium::Iterator::march(f32 uu) noexcept -> Medium_Interaction<render::Henyey_Greenstein_Phase_Function> {
    transmittance = {1.f};
    t_transmitted = 0.f;
    u = uu;

    while (true) {
        auto sigma_t = sigma_a + sigma_s;
        auto t_u = core::Exponential_Distribution{sigma_maj[0]}.sample(u);
        auto thin = sigma_maj[0] < core::epsilon<f32>;
        if (t_boundary <= t_cell && (thin || t_u >= t_boundary)) {
            update_transmittance(t_boundary);
            return {
                medium->phase,
                r.o,
                t_max,
                transmittance,
                {},
                {},
                {},
                {},
                {},
            };
        } else if (t_boundary > t_cell && (thin || t_u >= t_cell)) {
            update_transmittance(t_cell);
            update_majorant(t_boundary);
        } else {
            update_transmittance(t_u);
            auto density = std::as_const(medium->density)(r.o);
            return {
                medium->phase,
                r.o,
                t_transmitted,
                transmittance,
                density * sigma_a,
                density * sigma_s,
                core::max(sigma_maj - density * sigma_t, fv4{0.f}),
                sigma_maj,
                density * sigma_e,
            };
        }
    }
}

auto Heterogeneous_Medium::Iterator::update_majorant(f32 t) noexcept -> void {
    auto inside = medium->majorant.inside(cell);
    auto sign = core::sign(r.d);
    auto next_inside = medium->majorant.inside(cell + offset);
    if (inside && !next_inside) {
        t_cell = t_boundary;
        return;
    }

    auto bbox = medium->majorant.bounding_box(cell + offset);
    auto [t_enter, t_next, i_enter, i_next] = core::hitvi(r, bbox);
    if (t_next < -core::epsilon<f32> || t_enter > t_next + core::epsilon<f32>) {
        t_next = t_boundary;
        i_next = 0uz;
    }

    t_cell = t_next;
    cell += offset;
    offset = sign * iv3{i_next == 0, i_next == 1, i_next == 2};
    sigma_maj = medium->majorant[cell] * (sigma_a + sigma_s);
}

auto Heterogeneous_Medium::Iterator::update_transmittance(f32 t) noexcept -> void {
    t_transmitted += t;
    t_boundary -= t;
    t_cell -= t;
    r.o += t * r.d;
    auto exp = core::exp(-sigma_maj * t);
    u = 1.f - (1.f - u) / exp[0];
    transmittance *= exp;
}

Heterogeneous_Medium::Heterogeneous_Medium(
    render::Henyey_Greenstein_Phase_Function phase,
    render::Spectrum sigma_a, render::Spectrum sigma_s, render::Spectrum sigma_e,
    uzv3 dimensions, render::Volume density, f32 density_scale, core::recorder& recorder
) noexcept
: phase(phase), sigma_a(sigma_a), sigma_s(sigma_s), sigma_e(sigma_e), majorant(density.bounding_box(), dimensions), density(density), density_scale(density_scale) {
    auto storage = frw3{};
    storage.update<core::ordinary<32>, core::encoding::sf>({dimensions, 1});
    auto ctx = Context{density, majorant.bbox, storage};
    // core::pipenc{recorder, major, ctx}.dispatch(uv3{dimensions}, uv3{0});
    core::memonc{recorder}.copy(storage, majorant.storage);
    recorder.retain(std::move(storage));
}

auto Heterogeneous_Medium::begin(core::Ray_Context const& ctx, f32 t_max) const noexcept -> Iterator {
    return {*this, ctx, t_max};
}

}
