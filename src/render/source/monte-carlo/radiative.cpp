#include <metatron/render/monte-carlo/radiative.hpp>
#include <metatron/render/monte-carlo/context.hpp>
#include <metatron/core/stl/print.hpp>

namespace mtt::render {

struct Payload final {
    Accel_Interaction acc;
    Generator sampler;
    Emitter emitter;

    core::Ray_Context ctx;
    core::Ray_Differential diff;

    core::Ray shadow;
    Medium medium;
    core::Transform const* medium_to_render;

    f32 t{0.f};
    fv4 gamma{0.f};
    fv4 mis_d{0.f};
    fv4 mis_l{0.f};

    fv4 emission{0.f};
    fv4 beta{1.f};
    fv4 mis_s{1.f};
    fv4 mis_e{0.f};
    bool scattered{false};
    bool degraded{false};
};

[[mtt::visible]] auto miss(Monte_Carlo_Context& ctx, Payload& pay) noexcept -> void {
    pay.gamma = fv4{0.f};
    pay.scattered = false;
    pay.degraded = false;
    auto e_intr = pay.emitter.sample_infinite(pay.ctx, pay.sampler.generate_1d());
    if (e_intr.pdf > 0.f) {
        auto l_ctx = *e_intr.local_to_render ^ pay.ctx;
        auto l_intr = e_intr.light(l_ctx.r, l_ctx.lambda);
        pay.mis_e *= e_intr.pdf * l_intr.pdf;
        auto mis_w = core::guarded_div(1.f, core::avg(pay.mis_s + pay.mis_e));
        pay.emission += pay.beta * mis_w * l_intr.L;
    }
    pay.beta = fv4{0.f};
}

template<Intersectable Shape, Surfaced Material, Participating Medium>
[[mtt::visible]] auto hit(Monte_Carlo_Context& ctx, Payload& pay) noexcept -> void {
    auto& div = pay.acc.divider;
    auto& sp = pay.sampler;
    pay.gamma = fv4{0.f};
    pay.scattered = false;
    pay.degraded = false;

    auto lt = div->local_to_render;
    auto& shape = *div->shape.template data<Shape>();
    auto intr = shape(*lt ^ pay.ctx.r, *lt ^ pay.ctx.n, pay.acc.pos, pay.acc.primitive);
    intr.p = *lt | core::expand(intr.p, 1.f);
    intr.n = core::normalize(*lt | intr.n);
    pay.ctx.inside = pay.acc.pos[2] < 0.f;

    auto nee = [&](core::Ray_Context const& l_ctx, auto&& eval) {
        auto e_intr = pay.emitter.sample(l_ctx, sp.generate_1d());
        if (e_intr.pdf == 0.f) return;

        auto et = e_intr.local_to_render;
        auto l_intr = e_intr.light.sample(*et ^ l_ctx, sp.generate_2d());
        auto p_e = e_intr.pdf * l_intr.pdf;
        if (core::abs(p_e) < core::epsilon<f32>) return;
        l_intr.p = *et | core::expand(l_intr.p, 1.f);
        l_intr.wi = core::normalize(*et | core::expand(l_intr.wi, 0.f));
        l_intr.pdf = p_e;

        auto s_intr = eval(l_intr.wi);
        auto delta = e_intr.light.flags() & Light_Flags::delta;
        auto side = core::dot(l_intr.wi, l_ctx.n) < 0.f ? -1.f : 1.f;
        auto inside = (side > 0.f) == pay.ctx.inside;
        pay.medium = inside ? div->int_medium : div->ext_medium;
        pay.medium_to_render = inside ? div->int_to_render : div->ext_to_render;
        pay.shadow = {l_ctx.r.o + 0.001f * side * l_ctx.n, l_intr.wi};
        pay.t = core::isinf(l_intr.t) ? l_intr.t : core::length(l_intr.p - pay.shadow.o);
        pay.gamma = pay.beta * s_intr.f * l_intr.L / p_e;
        pay.mis_d = pay.mis_s * s_intr.pdf / p_e * f32(!delta);
        pay.mis_l = pay.mis_s;
    };

    auto& medium = *(pay.ctx.inside ? div->int_medium : div->ext_medium).template data<Medium>();
    auto medium_to_render = pay.ctx.inside ? div->int_to_render : div->ext_to_render;
    auto iter = medium.begin(*medium_to_render ^ pay.ctx, intr.t);
    auto surface = false;

    auto march = [&] -> bool {
        auto m_intr = iter.march(sp.generate_1d());
        surface = m_intr.t >= intr.t;
        auto spectra_pdf = surface ? m_intr.transmittance : m_intr.sigma_maj * m_intr.transmittance;
        auto flight_pdf = spectra_pdf[0];

        pay.beta *= m_intr.transmittance / flight_pdf;
        pay.mis_s *= spectra_pdf / flight_pdf;
        pay.mis_e *= spectra_pdf / flight_pdf;
        if (surface) return false;

        auto mis_a = core::guarded_div(1.f, core::avg(pay.mis_s));
        pay.emission += mis_a * pay.beta * m_intr.sigma_a * m_intr.sigma_e;

        auto p_a = core::guarded_div(m_intr.sigma_a[0], m_intr.sigma_maj[0]);
        auto p_s = core::guarded_div(m_intr.sigma_s[0], m_intr.sigma_maj[0]);
        auto p_n = core::guarded_div(m_intr.sigma_n[0], m_intr.sigma_maj[0]);
        auto distribution = core::Discrete_Distribution<3>{};
        distribution.update({p_a, p_s, p_n});
        auto mode = distribution.sample(sp.generate_1d());
        if (mode == 0uz) {
            pay.beta = fv4{0.f};
            return true;
        } else if (mode == 1uz) {
            pay.beta *= m_intr.sigma_s / p_s;
            pay.mis_s *= m_intr.sigma_s / m_intr.sigma_maj / p_s;
            auto point = fv3{*medium_to_render | core::expand(m_intr.p, 1.f)};
            auto l_ctx = pay.ctx;
            l_ctx.r.o = point;
            l_ctx.n = {};
            nee(l_ctx, [&](fv3 const& wi) {
                auto p_intr = m_intr.phase(pay.ctx.r.d, wi);
                return Bsdf_Interaction{p_intr.f, fv4{1.f}, wi, p_intr.pdf};
            });

            auto pt = core::Transform{};
            pt.transform = fm44{fq::from_rotation_between(-pay.ctx.r.d, {0.f, 1.f, 0.f})};
            pt.inv_transform = core::transpose(pt.transform);
            auto p_intr = m_intr.phase.sample(pt | pay.ctx, sp.generate_2d());
            p_intr.wi = core::normalize(pt ^ core::expand(p_intr.wi, 0.f));
            if (p_intr.pdf == 0.f) {
                pay.beta = fv4{0.f};
                return true;
            }
            pay.beta *= p_intr.f / p_intr.pdf;
            pay.mis_e = pay.mis_s / p_intr.pdf;

            auto translate = [](fv3 const& t) { auto m = fm44{1.f}; for (auto i = 0; i < 3; i++) m[i][3] = t[i]; return m; };
            auto rot = fm44{fq::from_rotation_between(core::normalize(pay.diff.r.d), p_intr.wi)};
            pay.diff = core::Transform{translate(point) | rot | translate(-pay.diff.r.o)} | pay.diff;
            pay.ctx.r = {point, p_intr.wi};
            pay.ctx.n = {};
            pay.scattered = true;
            return true;
        }

        pay.beta *= m_intr.sigma_n / p_n;
        pay.mis_s *= (m_intr.sigma_n / m_intr.sigma_maj) / p_n;
        pay.mis_e /= p_n;

        auto q = core::max(pay.beta * core::guarded_div(1.f, core::avg(pay.mis_s)));
        if (q < 1.f) {
            if (sp.generate_1d() > q) {
                pay.beta = fv4{0.f};
                return true;
            }
            pay.beta /= q;
        }
        return false;
    };

    if (ctx.integrator.data<Radiative_Integrator>()->volumetric)
        while (!surface)
            if (march()) return;

    auto flip_n = pay.ctx.inside ? -1.f : 1.f;
    intr.n *= flip_n;
    intr.dndu *= flip_n;
    intr.dndv *= flip_n;
    intr.tn = core::normalize(*lt | core::expand(intr.tn * flip_n, 0.f));
    intr.bn = core::normalize(*lt | core::expand(intr.bn * flip_n, 0.f));

    auto ldiff = *lt ^ pay.diff;
    auto l_intr = intr;
    l_intr.p = core::shrink(*lt ^ core::expand(l_intr.p, 1.f));
    l_intr.n = *lt ^ l_intr.n;
    auto duvdxy = grad(ldiff, l_intr);
    auto& material = *div->material.template data<Material>();
    auto mat_intr = material.sample(pay.ctx, l_intr.uv, duvdxy);

    if (core::max(mat_intr.emission) > core::epsilon<f32>) {
        pay.mis_e *= intr.pdf;
        auto mis_w = core::guarded_div(1.f, core::avg(pay.mis_s + pay.mis_e));
        pay.emission += mis_w * pay.beta * mat_intr.emission;
    }

    auto scatter = [&] {
        auto flags = mat_intr.bsdf.flags();
        auto tbn = core::transpose(fm33{intr.tn, intr.bn, intr.n});
        intr.n = tbn | mat_intr.normal;
        if (flags & Bsdf_Flags::bsdf_interface) {
            pay.ctx.r.o = intr.p - 0.001f * intr.n;
            return;
        }

        auto bt = core::Transform{};
        bt.transform = fm44{fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
        bt.inv_transform = core::transpose(bt.transform);

        auto specular = flags & Bsdf_Flags::specular;
        auto l_ctx = pay.ctx;
        l_ctx.r.o = intr.p;
        l_ctx.n = intr.n;
        if (!specular) nee(l_ctx, [&](fv3 const& wi) {
            auto wo = core::normalize(bt | core::expand(pay.ctx.r.d, 0.f));
            auto wl = core::normalize(bt | core::expand(wi, 0.f));
            auto b_intr = mat_intr.bsdf(wo, wl, -1.f);
            b_intr.f *= core::abs(core::dot(wi, intr.n));
            return b_intr;
        });

        auto cu = sp.generate_1d();
        auto du = sp.generate_2d();
        auto b_ctx = bt | pay.ctx;
        b_ctx.r.d = core::normalize(b_ctx.r.d);
        auto b_intr = mat_intr.bsdf.sample(b_ctx, {cu, du[0], du[1]});
        if (b_intr.pdf == 0.f) {
            pay.beta = fv4{0.f};
            return;
        }

        auto crossed = b_intr.wi[1] < 0.f;
        auto trace_n = (crossed ? -1.f : 1.f) * intr.n;
        b_intr.f *= core::abs(core::unit_to_cos_theta(b_intr.wi));
        b_intr.wi = core::normalize(bt ^ core::expand(b_intr.wi, 0.f));

        auto l_wi = core::normalize(core::shrink(*lt ^ core::expand(b_intr.wi, 0.f)));
        pay.diff = *lt | propagate(ldiff, l_intr, duvdxy, l_wi, b_intr.eta);
        pay.ctx.r = {intr.p + 0.001f * trace_n, b_intr.wi};
        pay.ctx.n = trace_n;
        pay.beta *= b_intr.f / b_intr.pdf;
        pay.mis_e = specular ? fv4{0.f} : pay.mis_s / b_intr.pdf;
        pay.scattered = true;
        pay.degraded = mat_intr.degraded && crossed && !core::constant(pay.ctx.lambda);
    };
    scatter();
}

template<Intersectable Shape, Participating Medium>
[[mtt::visible]] auto track(Monte_Carlo_Context& ctx, Payload& pay) noexcept -> void {
    auto& sp = pay.sampler;
    auto& r = pay.shadow;
    auto n = fv3{0.f};

    auto div = pay.acc.divider;
    auto lt = div->local_to_render;
    auto& shape = *div->shape.template data<Shape>();
    auto intr = shape(*lt ^ r, *lt ^ n, pay.acc.pos, pay.acc.primitive);
    intr.p = *lt | core::expand(intr.p, 1.f);
    intr.n = core::normalize(*lt | intr.n);

    auto l_ctx = core::Ray_Context{r, n, pay.ctx.lambda, pay.acc.pos[2] < 0.f};
    auto& medium = *pay.medium.template data<Medium>();
    auto seg = core::min(intr.t, pay.t);
    auto iter = medium.begin(*pay.medium_to_render ^ l_ctx, seg);
    auto boundary = false;

    auto march = [&] -> bool {
        auto m_intr = iter.march(sp.generate_1d());
        boundary = m_intr.t >= seg;
        auto spectra_pdf = boundary ? m_intr.transmittance : m_intr.sigma_maj * m_intr.transmittance;
        auto flight_pdf = spectra_pdf[0];

        pay.gamma *= m_intr.transmittance / flight_pdf;
        pay.mis_d *= spectra_pdf / flight_pdf;
        pay.mis_l *= spectra_pdf / flight_pdf;
        if (boundary) return false;

        auto p_n = core::guarded_div(m_intr.sigma_n[0], m_intr.sigma_maj[0]);
        if (sp.generate_1d() >= p_n) {
            pay.gamma = fv4{0.f};
            return true;
        }
        pay.gamma *= m_intr.sigma_n / p_n;
        pay.mis_d *= m_intr.sigma_n / m_intr.sigma_maj / p_n;
        pay.mis_l *= m_intr.sigma_n / m_intr.sigma_maj / p_n;
        return false;
    };

    while (!boundary)
        if (march()) return;

    if (intr.t >= pay.t - 0.001f) {
        auto mis_u = core::guarded_div(1.f, core::avg(pay.mis_d + pay.mis_l));
        pay.emission += pay.gamma * mis_u;
        pay.gamma = fv4{0.f};
        return;
    }

    auto face_n = l_ctx.inside ? -intr.n : intr.n;
    r.o = intr.p - 0.001f * face_n;
    pay.t -= intr.t;
    pay.medium = l_ctx.inside ? div->ext_medium : div->int_medium;
    pay.medium_to_render = l_ctx.inside ? div->ext_to_render : div->int_to_render;
}

[[mtt::kernel]] auto sample(Monte_Carlo_Context& ctx, uv3 px) noexcept -> void {
    auto size = uv2{ctx.film->image.size()};
    if (px[0] >= size[0] || px[1] >= size[1]) return;

    auto sp = Generator{ctx.sampler, {{}, uv2{px}, size, ctx.sample_index, ctx.film->spp, 0, ctx.seed}};
    sp.start();
    auto fixel = (*ctx.film)(ctx.filter, uzv2{px}, sp.generate_pixel_2d());
    auto s = Camera{}.sample(ctx.lens, fixel.position, fixel.dxdy, sp.generate_2d());
    s.ray_differential = *ctx.camera ^ s.ray_differential;
    auto spec = Stochastic_Spectrum{sp.generate_1d()};

    auto pay = Payload{};
    pay.sampler = sp;
    pay.emitter = ctx.emitter;
    pay.ctx.r = s.ray_differential.r;
    pay.ctx.lambda = spec.lambda;
    pay.diff = s.ray_differential;

    for (auto bounce = 0u; bounce < ctx.film->depth; bounce += pay.scattered) {
        auto q = core::max(pay.beta * core::guarded_div(1.f, core::avg(pay.mis_s)));
        if (q < 1.f) {
            if (pay.sampler.generate_1d() > q) break;
            pay.beta /= q;
        }

        pay.acc = ctx.accel(pay.ctx.r, pay.ctx.n, Accel_Flags(0), {0.f, core::inf<f32>});
        // reorder
        auto div = pay.acc.divider;
        if (!div) {
            core::cartesian{}([&] { miss(ctx, pay); });
            break;
        }
        auto medium = pay.acc.pos[2] < 0.f ? div->int_medium : div->ext_medium;
        core::cartesian{}([&]<typename... Ts>() { hit<Ts...>(ctx, pay); }, div->shape, div->material, medium);

        if (pay.gamma != fv4{0.f}) {
            auto occlusion = Accel_Flags(Accel_Flags::hit_first | Accel_Flags::skip_interface);
            auto blocker = ctx.accel(pay.shadow, fv3{0.f}, occlusion, {0.f, pay.t - 0.001f});
            if (blocker.divider) pay.gamma = fv4{0.f};
        }
        if (!ctx.integrator.data<Radiative_Integrator>()->volumetric && pay.gamma != fv4{0.f}) {
            auto mis_u = core::guarded_div(1.f, core::avg(pay.mis_d + pay.mis_l));
            pay.emission += pay.gamma * mis_u;
            pay.gamma = fv4{0.f};
        }
        while (pay.gamma != fv4{0.f}) {
            pay.acc = pay.t > 0.001f ? ctx.accel(pay.shadow, fv3{0.f}, Accel_Flags::only_interface, {0.f, core::inf<f32>}) : Accel_Interaction{};
            auto div = pay.acc.divider;
            if (div) core::cartesian{}([&]<typename Shape, typename Medium>() { track<Shape, Medium>(ctx, pay); }, div->shape, pay.medium);
            else {
                auto mis_u = core::guarded_div(1.f, core::avg(pay.mis_d + pay.mis_l));
                pay.emission += pay.gamma * mis_u;
                pay.gamma = fv4{0.f};
            }
        }

        if (pay.degraded) {
            pay.emission = fv4{pay.emission[0]};
            pay.beta = fv4{pay.beta[0]};
            pay.mis_s = fv4{pay.mis_s[0]};
            pay.mis_e = fv4{pay.mis_e[0]};
            pay.ctx.lambda = fv4{pay.ctx.lambda[0]};
        }
        if (pay.beta == fv4{0.f}) break;
    }

    auto Li = Stochastic_Spectrum{pay.ctx.lambda, pay.emission};
    Li.value /= s.pdf;
    if (!core::isfinite(Li.value))
        core::abort("invalid value appears in pixel {} sample {}", px, ctx.sample_index);
    fixel = Li;
}

auto Radiative_Integrator::trace(core::recorder& r, Monte_Carlo_Context& ctx) const noexcept -> void {
    core::pipenc(r, sample, ctx).dispatch({uv2{ctx.film->image.size()}, 1});
}

}
