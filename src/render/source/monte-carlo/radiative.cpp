#include <metatron/render/monte-carlo/radiative.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::monte_carlo {
    struct Radiative_Integrator::Payload final {
        accel::Interaction acc;
        sampler::proxy::Sampler sampler;
        emitter::Emitter emitter;

        math::Context ctx;
        math::Ray_Differential diff;

        math::Ray shadow;
        f32 t{0.f};
        fv4 gamma{0.f};
        fv4 mis_d{0.f};

        fv4 emission{0.f};
        fv4 beta{1.f};
        fv4 mis_s{1.f};
        fv4 mis_e{0.f};
        bool scattered{false};
        bool degraded{false};
    };

    auto Radiative_Integrator::hit(ref<Payload> payload) const noexcept -> void {
        auto& ctx = payload.ctx;
        auto& div = payload.acc.divider;
        auto& sp = payload.sampler;
        payload.gamma = fv4{0.f};
        payload.scattered = false;
        payload.degraded = false;

        if (math::isinf(payload.acc.pos[3])) {
            auto e_intr = payload.emitter.sample_infinite(ctx, sp.generate_1d());
            if (e_intr.pdf > 0.f) {
                auto l_ctx = e_intr.local_to_render ^ ctx;
                auto l_intr = e_intr.light(l_ctx.r, l_ctx.lambda);
                payload.mis_e *= e_intr.pdf * l_intr.pdf;
                auto mis_w = math::guarded_div(1.f, math::avg(payload.mis_s + payload.mis_e));
                payload.emission += payload.beta * mis_w * l_intr.L;
            }
            payload.beta = fv4{0.f};
            return;
        }

        auto lt = div->local_to_render;
        auto intr = div->shape(lt ^ ctx.r, lt ^ ctx.n, payload.acc.pos, payload.acc.primitive);
        intr.p = lt | math::expand(intr.p, 1.f);
        intr.n = math::normalize(lt | intr.n);
        ctx.inside = math::dot(-ctx.r.d, intr.n) < 0.f;

        auto nee = [&](cref<math::Context> l_ctx, auto&& eval) {
            auto e_intr = payload.emitter.sample(l_ctx, sp.generate_1d());
            if (e_intr.pdf == 0.f) return;

            auto et = e_intr.local_to_render;
            auto l_intr = e_intr.light.sample(et ^ l_ctx, sp.generate_2d());
            auto p_e = e_intr.pdf * l_intr.pdf;
            if (math::abs(p_e) < math::epsilon<f32>) return;
            l_intr.p = et | math::expand(l_intr.p, 1.f);
            l_intr.wi = math::normalize(et | math::expand(l_intr.wi, 0.f));
            l_intr.pdf = p_e;

            auto s_intr = eval(l_intr.wi);
            auto delta = e_intr.light.flags() & light::Flags::delta;
            auto side = math::dot(l_intr.wi, l_ctx.n) < 0.f ? -1.f : 1.f;
            payload.shadow = {l_ctx.r.o + 0.001f * side * l_ctx.n, l_intr.wi};
            payload.t = math::isinf(l_intr.t) ? l_intr.t : math::length(l_intr.p - payload.shadow.o);
            payload.gamma = payload.beta * s_intr.f * l_intr.L / p_e;
            payload.mis_d = payload.mis_s * s_intr.pdf / p_e * f32(!delta);
        };

        auto medium = ctx.inside ? div->int_medium : div->ext_medium;
        auto medium_to_render = ctx.inside ? div->int_to_render : div->ext_to_render;
        auto iter = medium.begin(medium_to_render ^ ctx, intr.t);
        auto surface = false;

        auto march = [&] -> bool {
            auto m_intr = iter.march(sp.generate_1d());
            surface = m_intr.t >= intr.t;
            auto spectra_pdf = surface
            ? m_intr.transmittance
            : m_intr.sigma_maj * m_intr.transmittance;
            auto flight_pdf = spectra_pdf[0];

            payload.beta *= m_intr.transmittance / flight_pdf;
            payload.mis_s *= spectra_pdf / flight_pdf;
            payload.mis_e *= spectra_pdf / flight_pdf;
            if (surface) return false;

            auto mis_a = math::guarded_div(1.f, math::avg(payload.mis_s));
            payload.emission += mis_a * payload.beta * m_intr.sigma_a * m_intr.sigma_e;

            auto p_a = math::guarded_div(m_intr.sigma_a[0], m_intr.sigma_maj[0]);
            auto p_s = math::guarded_div(m_intr.sigma_s[0], m_intr.sigma_maj[0]);
            auto p_n = math::guarded_div(m_intr.sigma_n[0], m_intr.sigma_maj[0]);
            auto mode = math::Discrete_Distribution<3>{{p_a, p_s, p_n}}.sample(sp.generate_1d());
            if (mode == 0uz) {
                payload.beta = fv4{0.f};
                return true;
            } else if (mode == 1uz) {
                payload.beta *= m_intr.sigma_s / p_s;
                payload.mis_s *= m_intr.sigma_s / m_intr.sigma_maj / p_s;
                auto point = fv3{medium_to_render | math::expand(m_intr.p, 1.f)};
                auto l_ctx = ctx;
                l_ctx.r.o = point;
                l_ctx.n = {};
                nee(l_ctx, [&](cref<fv3> wi) {
                    auto p_intr = m_intr.phase(ctx.r.d, wi);
                    return bsdf::Interaction{p_intr.f, fv4{1.f}, wi, p_intr.pdf};
                });

                auto pt = math::Transform{};
                pt.transform = fm44{fq::from_rotation_between(-ctx.r.d, {0.f, 1.f, 0.f})};
                pt.inv_transform = math::transpose(pt.transform);
                auto p_intr = m_intr.phase.sample(pt | ctx, sp.generate_2d());
                p_intr.wi = math::normalize(pt ^ math::expand(p_intr.wi, 0.f));
                if (p_intr.pdf == 0.f) {
                    payload.beta = fv4{0.f};
                    return true;
                }
                payload.beta *= p_intr.f / p_intr.pdf;
                payload.mis_e = payload.mis_s / p_intr.pdf;

                auto translate = [](cref<fv3> t) { auto m = fm44{1.f}; for (auto i = 0; i < 3; i++) m[i][3] = t[i]; return m; };
                auto rot = fm44{fq::from_rotation_between(math::normalize(payload.diff.r.d), p_intr.wi)};
                payload.diff = math::Transform{translate(point) | rot | translate(-payload.diff.r.o)} | payload.diff;
                ctx.r = {point, p_intr.wi};
                ctx.n = {};
                payload.scattered = true;
                return true;
            }

            payload.beta *= m_intr.sigma_n / p_n;
            payload.mis_s *= (m_intr.sigma_n / m_intr.sigma_maj) / p_n;
            payload.mis_e /= p_n;

            auto q = math::max(payload.beta * math::guarded_div(1.f, math::avg(payload.mis_s)));
            if (q < 1.f) {
                if (sp.generate_1d() > q) { payload.beta = fv4{0.f}; return true; }
                payload.beta /= q;
            }
            return false;
        };
        while (!surface) if (march()) return;

        auto flip_n = ctx.inside ? -1.f : 1.f;
        intr.n *= flip_n; intr.dndu *= flip_n; intr.dndv *= flip_n;
        intr.tn = math::normalize(lt | math::expand(intr.tn * flip_n, 0.f));
        intr.bn = math::normalize(lt | math::expand(intr.bn * flip_n, 0.f));

        auto ldiff = lt ^ payload.diff;
        auto l_intr = intr;
        l_intr.p = math::shrink(lt ^ math::expand(l_intr.p, 1.f));
        l_intr.n = lt ^ l_intr.n;
        auto tcoord = texture::grad(ldiff, l_intr);
        auto mat_intr = div->material.sample(ctx, tcoord);

        if (math::max(mat_intr.emission) > math::epsilon<f32>) {
            payload.mis_e *= intr.pdf;
            auto mis_w = math::guarded_div(1.f, math::avg(payload.mis_s + payload.mis_e));
            payload.emission += mis_w * payload.beta * mat_intr.emission;
        }

        auto scatter = [&] {
            auto flags = mat_intr.bsdf.flags();
            auto tbn = math::transpose(fm33{intr.tn, intr.bn, intr.n});
            intr.n = tbn | mat_intr.normal;
            if (flags & bsdf::Flags::interface) { ctx.r.o = intr.p - 0.001f * intr.n; return; }

            auto bt = math::Transform{};
            bt.transform = fm44{fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
            bt.inv_transform = math::transpose(bt.transform);

            auto specular = flags & bsdf::Flags::specular;
            auto l_ctx = ctx; l_ctx.r.o = intr.p; l_ctx.n = intr.n;
            if (!specular) nee(l_ctx, [&](cref<fv3> wi) {
                auto wo = math::normalize(bt | math::expand(ctx.r.d, 0.f));
                auto wl = math::normalize(bt | math::expand(wi, 0.f));
                auto b_intr = mat_intr.bsdf(wo, wl);
                b_intr.f *= math::abs(math::dot(wi, intr.n));
                return b_intr;
            });

            auto cu = sp.generate_1d();
            auto du = sp.generate_2d();
            auto b_ctx = bt | ctx;
            b_ctx.r.d = math::normalize(b_ctx.r.d);
            auto b_intr = mat_intr.bsdf.sample(b_ctx, {cu, du[0], du[1]});
            if (b_intr.pdf == 0.f) { payload.beta = fv4{0.f}; return; }

            auto crossed = b_intr.wi[1] < 0.f;
            auto trace_n = (crossed ? -1.f : 1.f) * intr.n;
            b_intr.f *= math::abs(math::unit_to_cos_theta(b_intr.wi));
            b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));

            auto l_wi = math::normalize(math::shrink(lt ^ math::expand(b_intr.wi, 0.f)));
            payload.diff = lt | texture::propagate(ldiff, l_intr, tcoord, l_wi, b_intr.eta);
            ctx.r = {intr.p + 0.001f * trace_n, b_intr.wi};
            ctx.n = trace_n;
            payload.beta *= b_intr.f / b_intr.pdf;
            payload.mis_e = specular ? fv4{0.f} : payload.mis_s / b_intr.pdf;
            payload.scattered = true;
            payload.degraded = mat_intr.degraded && crossed && !math::constant(ctx.lambda);
        };
        scatter();
    }

    auto Radiative_Integrator::track(ref<Payload> payload, cref<accel::Acceleration> accel) const noexcept -> void {
        if (payload.gamma == fv4{0.f}) return;
        auto& sp = payload.sampler;
        auto r = payload.shadow;
        auto n = fv3{0.f};
        auto t = payload.t;
        auto mis_l = payload.mis_s;
        auto boundary = false;

        auto march = [&](ref<media::Iterator> iter, f32 seg) -> bool {
            auto m_intr = iter.march(sp.generate_1d());
            boundary = m_intr.t >= seg;
            auto spectra_pdf = boundary
            ? m_intr.transmittance
            : m_intr.sigma_maj * m_intr.transmittance;
            auto flight_pdf = spectra_pdf[0];

            payload.gamma *= m_intr.transmittance / flight_pdf;
            payload.mis_d *= spectra_pdf / flight_pdf;
            mis_l *= spectra_pdf / flight_pdf;
            if (boundary) return false;

            auto p_n = math::guarded_div(m_intr.sigma_n[0], m_intr.sigma_maj[0]);
            if (sp.generate_1d() >= p_n) { payload.gamma = fv4{0.f}; return true; }
            payload.gamma *= m_intr.sigma_n / p_n;
            payload.mis_d *= m_intr.sigma_n / m_intr.sigma_maj / p_n;
            mis_l *= m_intr.sigma_n / m_intr.sigma_maj / p_n;
            return false;
        };

        while (t > 0.001f) {
            auto acc = accel(r, n);
            if (math::isinf(acc.pos[3])) break;

            auto div = acc.divider;
            auto lt = div->local_to_render;
            auto intr = div->shape(lt ^ r, lt ^ n, acc.pos, acc.primitive);
            intr.p = lt | math::expand(intr.p, 1.f);
            intr.n = math::normalize(lt | intr.n);

            auto l_ctx = math::Context{r, n, payload.ctx.lambda, math::dot(-r.d, intr.n) < 0.f};
            auto medium = l_ctx.inside ? div->int_medium : div->ext_medium;
            auto medium_to_render = l_ctx.inside ? div->int_to_render : div->ext_to_render;
            auto seg = math::min(intr.t, t);
            auto iter = medium.begin(medium_to_render ^ l_ctx, seg);
            boundary = false;
            while (!boundary) if (march(iter, seg)) return;

            if (intr.t >= t - 0.001f) break;
            if (!(div->material.flags() & material::Flags::interface)) {
                payload.gamma = fv4{0.f};
                return;
            }

            auto face_n = l_ctx.inside ? -intr.n : intr.n;
            r.o = intr.p - 0.001f * face_n;
            t -= intr.t;
        }

        auto mis_u = math::guarded_div(1.f, math::avg(payload.mis_d + mis_l));
        payload.emission += payload.gamma * mis_u;
    }

    auto Radiative_Integrator::sample(ref<Context> ctx, cref<uzv2> px) const noexcept -> void {
        auto size = uzv2{ctx.film->image.size};
        auto sp = sampler::proxy::Sampler{ctx.sampler, {{}, px, size, ctx.sample_index, ctx.film->spp, 0, ctx.seed}};
        sp.start();
        auto fixel = ctx.film(ctx.filter, px, sp.generate_pixel_2d());
        auto s = photo::Camera{}.sample(ctx.lens, fixel.position, fixel.dxdy, sp.generate_2d());
        s.ray_differential = ctx.camera ^ s.ray_differential;
        auto spec = spectra::Stochastic_Spectrum{sp.generate_1d()};

        auto payload = Payload{};
        payload.sampler = sp;
        payload.emitter = ctx.emitter;
        payload.ctx.r = s.ray_differential.r;
        payload.ctx.lambda = spec.lambda;
        payload.diff = s.ray_differential;

        for (auto bounce = 0u; bounce < ctx.film->depth; bounce += payload.scattered) {
            auto q = math::max(payload.beta * math::guarded_div(1.f, math::avg(payload.mis_s)));
            if (q < 1.f) {
                if (payload.sampler.generate_1d() > q) break;
                payload.beta /= q;
            }

            payload.acc = ctx.accel(payload.ctx.r, payload.ctx.n);
            // reorder
            hit(payload);
            track(payload, ctx.accel);

            if (payload.degraded) {
                payload.emission = fv4{payload.emission[0]};
                payload.beta = fv4{payload.beta[0]};
                payload.mis_s = fv4{payload.mis_s[0]};
                payload.mis_e = fv4{payload.mis_e[0]};
                payload.ctx.lambda = fv4{payload.ctx.lambda[0]};
            }
            if (payload.beta == fv4{0.f}) break;
        }

        auto Li = spectra::Stochastic_Spectrum{payload.ctx.lambda, payload.emission};
        Li.value /= s.pdf;
        if (!math::isfinite(Li.value))
            stl::abort("invalid value appears in pixel {} sample {}", px, ctx.sample_index);
        fixel = Li;
    }

    Radiative_Integrator::Radiative_Integrator(cref<Descriptor>) noexcept {}

    auto Radiative_Integrator::trace(ref<Context> ctx) const noexcept -> void {
        auto size = uzv2{ctx.film->image.size};
        stl::scheduler::sync_parallel(size, [&](auto&& px) { sample(ctx, px); });
    }
}
