#include <metatron/render/monte-carlo/radiative.hpp>
#include <metatron/render/monte-carlo/context.hpp>

namespace mtt::monte_carlo {
    struct Payload final {
        accel::Interaction acc;
        sampler::proxy::Sampler sampler;
        emitter::Emitter emitter;

        math::Context ctx;
        math::Ray_Differential diff;

        math::Ray shadow;
        media::Medium medium;
        math::proxy::Transform medium_to_render;

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

    [[mtt::visible]] auto miss(Context& ctx, Payload& pay) noexcept -> void {
        pay.gamma = fv4{0.f};
        pay.scattered = false;
        pay.degraded = false;
        auto e_intr = pay.emitter.sample_infinite(pay.ctx, pay.sampler.generate_1d());
        if (e_intr.pdf > 0.f) {
            auto l_ctx = e_intr.local_to_render ^ pay.ctx;
            auto l_intr = e_intr.light(l_ctx.r, l_ctx.lambda);
            pay.mis_e *= e_intr.pdf * l_intr.pdf;
            auto mis_w = math::guarded_div(1.f, math::avg(pay.mis_s + pay.mis_e));
            pay.emission += pay.beta * mis_w * l_intr.L;
        }
        pay.beta = fv4{0.f};
    }

    template<shape::Intersectable Shape, material::Surfaced Material, media::Participating Medium>
    [[mtt::visible]] auto hit(Context& ctx, Payload& pay) noexcept -> void {
        auto& div = pay.acc.divider;
        auto& sp = pay.sampler;
        pay.gamma = fv4{0.f};
        pay.scattered = false;
        pay.degraded = false;

        auto lt = div->local_to_render;
        auto& shape = *div->shape.idx.template data<Shape>();
        auto intr = shape(lt ^ pay.ctx.r, lt ^ pay.ctx.n, pay.acc.pos, pay.acc.primitive);
        intr.p = lt | math::expand(intr.p, 1.f);
        intr.n = math::normalize(lt | intr.n);
        pay.ctx.inside = pay.acc.pos[2] < 0.f;

        auto nee = [&](math::Context const& l_ctx, auto&& eval) {
            auto e_intr = pay.emitter.sample(l_ctx, sp.generate_1d());
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
            auto inside = (side > 0.f) == pay.ctx.inside;
            pay.medium = inside ? div->int_medium : div->ext_medium;
            pay.medium_to_render = inside ? div->int_to_render : div->ext_to_render;
            pay.shadow = {l_ctx.r.o + 0.001f * side * l_ctx.n, l_intr.wi};
            pay.t = math::isinf(l_intr.t) ? l_intr.t : math::length(l_intr.p - pay.shadow.o);
            pay.gamma = pay.beta * s_intr.f * l_intr.L / p_e;
            pay.mis_d = pay.mis_s * s_intr.pdf / p_e * f32(!delta);
            pay.mis_l = pay.mis_s;
        };

        auto& medium = *(pay.ctx.inside ? div->int_medium : div->ext_medium).template data<Medium>();
        auto medium_to_render = pay.ctx.inside ? div->int_to_render : div->ext_to_render;
        auto iter = medium.begin(medium_to_render ^ pay.ctx, intr.t);
        auto surface = false;

        auto march = [&] -> bool {
            auto m_intr = iter.march(sp.generate_1d());
            surface = m_intr.t >= intr.t;
            auto spectra_pdf = surface
            ? m_intr.transmittance
            : m_intr.sigma_maj * m_intr.transmittance;
            auto flight_pdf = spectra_pdf[0];

            pay.beta *= m_intr.transmittance / flight_pdf;
            pay.mis_s *= spectra_pdf / flight_pdf;
            pay.mis_e *= spectra_pdf / flight_pdf;
            if (surface) return false;

            auto mis_a = math::guarded_div(1.f, math::avg(pay.mis_s));
            pay.emission += mis_a * pay.beta * m_intr.sigma_a * m_intr.sigma_e;

            auto p_a = math::guarded_div(m_intr.sigma_a[0], m_intr.sigma_maj[0]);
            auto p_s = math::guarded_div(m_intr.sigma_s[0], m_intr.sigma_maj[0]);
            auto p_n = math::guarded_div(m_intr.sigma_n[0], m_intr.sigma_maj[0]);
            auto mode = math::Discrete_Distribution<3>{{p_a, p_s, p_n}}.sample(sp.generate_1d());
            if (mode == 0uz) {
                pay.beta = fv4{0.f};
                return true;
            } else if (mode == 1uz) {
                pay.beta *= m_intr.sigma_s / p_s;
                pay.mis_s *= m_intr.sigma_s / m_intr.sigma_maj / p_s;
                auto point = fv3{medium_to_render | math::expand(m_intr.p, 1.f)};
                auto l_ctx = pay.ctx;
                l_ctx.r.o = point;
                l_ctx.n = {};
                nee(l_ctx, [&](fv3 const& wi) {
                    auto p_intr = m_intr.phase(pay.ctx.r.d, wi);
                    return bsdf::Interaction{p_intr.f, fv4{1.f}, wi, p_intr.pdf};
                });

                auto pt = math::Transform{};
                pt.transform = fm44{fq::from_rotation_between(-pay.ctx.r.d, {0.f, 1.f, 0.f})};
                pt.inv_transform = math::transpose(pt.transform);
                auto p_intr = m_intr.phase.sample(pt | pay.ctx, sp.generate_2d());
                p_intr.wi = math::normalize(pt ^ math::expand(p_intr.wi, 0.f));
                if (p_intr.pdf == 0.f) {
                    pay.beta = fv4{0.f};
                    return true;
                }
                pay.beta *= p_intr.f / p_intr.pdf;
                pay.mis_e = pay.mis_s / p_intr.pdf;

                auto translate = [](fv3 const& t) { auto m = fm44{1.f}; for (auto i = 0; i < 3; i++) m[i][3] = t[i]; return m; };
                auto rot = fm44{fq::from_rotation_between(math::normalize(pay.diff.r.d), p_intr.wi)};
                pay.diff = math::Transform{translate(point) | rot | translate(-pay.diff.r.o)} | pay.diff;
                pay.ctx.r = {point, p_intr.wi};
                pay.ctx.n = {};
                pay.scattered = true;
                return true;
            }

            pay.beta *= m_intr.sigma_n / p_n;
            pay.mis_s *= (m_intr.sigma_n / m_intr.sigma_maj) / p_n;
            pay.mis_e /= p_n;

            auto q = math::max(pay.beta * math::guarded_div(1.f, math::avg(pay.mis_s)));
            if (q < 1.f) {
                if (sp.generate_1d() > q) { pay.beta = fv4{0.f}; return true; }
                pay.beta /= q;
            }
            return false;
        };
        if (ctx.integrator.idx.data<Radiative_Integrator>()->volumetric) while (!surface) if (march()) return;

        auto flip_n = pay.ctx.inside ? -1.f : 1.f;
        intr.n *= flip_n; intr.dndu *= flip_n; intr.dndv *= flip_n;
        intr.tn = math::normalize(lt | math::expand(intr.tn * flip_n, 0.f));
        intr.bn = math::normalize(lt | math::expand(intr.bn * flip_n, 0.f));

        auto ldiff = lt ^ pay.diff;
        auto l_intr = intr;
        l_intr.p = math::shrink(lt ^ math::expand(l_intr.p, 1.f));
        l_intr.n = lt ^ l_intr.n;
        auto tcoord = texture::grad(ldiff, l_intr);
        auto& material = *div->material.template data<Material>();
        auto mat_intr = material.sample(pay.ctx, tcoord);

        if (math::max(mat_intr.emission) > math::epsilon<f32>) {
            pay.mis_e *= intr.pdf;
            auto mis_w = math::guarded_div(1.f, math::avg(pay.mis_s + pay.mis_e));
            pay.emission += mis_w * pay.beta * mat_intr.emission;
        }

        auto scatter = [&] {
            auto flags = mat_intr.bsdf.flags();
            auto tbn = math::transpose(fm33{intr.tn, intr.bn, intr.n});
            intr.n = tbn | mat_intr.normal;
            if (flags & bsdf::Flags::interface) { pay.ctx.r.o = intr.p - 0.001f * intr.n; return; }

            auto bt = math::Transform{};
            bt.transform = fm44{fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
            bt.inv_transform = math::transpose(bt.transform);

            auto specular = flags & bsdf::Flags::specular;
            auto l_ctx = pay.ctx; l_ctx.r.o = intr.p; l_ctx.n = intr.n;
            if (!specular) nee(l_ctx, [&](fv3 const& wi) {
                auto wo = math::normalize(bt | math::expand(pay.ctx.r.d, 0.f));
                auto wl = math::normalize(bt | math::expand(wi, 0.f));
                auto b_intr = mat_intr.bsdf(wo, wl, -1.f);
                b_intr.f *= math::abs(math::dot(wi, intr.n));
                return b_intr;
            });

            auto cu = sp.generate_1d();
            auto du = sp.generate_2d();
            auto b_ctx = bt | pay.ctx;
            b_ctx.r.d = math::normalize(b_ctx.r.d);
            auto b_intr = mat_intr.bsdf.sample(b_ctx, {cu, du[0], du[1]});
            if (b_intr.pdf == 0.f) { pay.beta = fv4{0.f}; return; }

            auto crossed = b_intr.wi[1] < 0.f;
            auto trace_n = (crossed ? -1.f : 1.f) * intr.n;
            b_intr.f *= math::abs(math::unit_to_cos_theta(b_intr.wi));
            b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));

            auto l_wi = math::normalize(math::shrink(lt ^ math::expand(b_intr.wi, 0.f)));
            pay.diff = lt | texture::propagate(ldiff, l_intr, tcoord, l_wi, b_intr.eta);
            pay.ctx.r = {intr.p + 0.001f * trace_n, b_intr.wi};
            pay.ctx.n = trace_n;
            pay.beta *= b_intr.f / b_intr.pdf;
            pay.mis_e = specular ? fv4{0.f} : pay.mis_s / b_intr.pdf;
            pay.scattered = true;
            pay.degraded = mat_intr.degraded && crossed && !math::constant(pay.ctx.lambda);
        };
        scatter();
    }

    template<shape::Intersectable Shape, media::Participating Medium>
    [[mtt::visible]] auto track(Context& ctx, Payload& pay) noexcept -> void {
        auto& sp = pay.sampler;
        auto& r = pay.shadow;
        auto n = fv3{0.f};

        auto div = pay.acc.divider;
        auto lt = div->local_to_render;
        auto& shape = *div->shape.idx.template data<Shape>();
        auto intr = shape(lt ^ r, lt ^ n, pay.acc.pos, pay.acc.primitive);
        intr.p = lt | math::expand(intr.p, 1.f);
        intr.n = math::normalize(lt | intr.n);

        auto l_ctx = math::Context{r, n, pay.ctx.lambda, pay.acc.pos[2] < 0.f};
        auto& medium = *pay.medium.template data<Medium>();
        auto seg = math::min(intr.t, pay.t);
        auto iter = medium.begin(pay.medium_to_render ^ l_ctx, seg);
        auto boundary = false;

        auto march = [&] -> bool {
            auto m_intr = iter.march(sp.generate_1d());
            boundary = m_intr.t >= seg;
            auto spectra_pdf = boundary
            ? m_intr.transmittance
            : m_intr.sigma_maj * m_intr.transmittance;
            auto flight_pdf = spectra_pdf[0];

            pay.gamma *= m_intr.transmittance / flight_pdf;
            pay.mis_d *= spectra_pdf / flight_pdf;
            pay.mis_l *= spectra_pdf / flight_pdf;
            if (boundary) return false;

            auto p_n = math::guarded_div(m_intr.sigma_n[0], m_intr.sigma_maj[0]);
            if (sp.generate_1d() >= p_n) { pay.gamma = fv4{0.f}; return true; }
            pay.gamma *= m_intr.sigma_n / p_n;
            pay.mis_d *= m_intr.sigma_n / m_intr.sigma_maj / p_n;
            pay.mis_l *= m_intr.sigma_n / m_intr.sigma_maj / p_n;
            return false;
        };
        while (!boundary) if (march()) return;

        if (intr.t >= pay.t - 0.001f) {
            auto mis_u = math::guarded_div(1.f, math::avg(pay.mis_d + pay.mis_l));
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

    [[mtt::kernel]] auto sample(Context& ctx, uv3 px) noexcept -> void {
        auto size = uv2{ctx.film->image.size};
        if (px[0] >= size[0] || px[1] >= size[1]) return;

        auto sp = sampler::proxy::Sampler{ctx.sampler, {{}, uv2{px}, size, ctx.sample_index, ctx.film->spp, 0, ctx.seed}};
        sp.start();
        auto fixel = ctx.film(ctx.filter, uzv2{px}, sp.generate_pixel_2d());
        auto s = photo::Camera{}.sample(ctx.lens, fixel.position, fixel.dxdy, sp.generate_2d());
        s.ray_differential = ctx.camera ^ s.ray_differential;
        auto spec = spectra::Stochastic_Spectrum{sp.generate_1d()};

        auto pay = Payload{};
        pay.sampler = sp;
        pay.emitter = ctx.emitter;
        pay.ctx.r = s.ray_differential.r;
        pay.ctx.lambda = spec.lambda;
        pay.diff = s.ray_differential;

        for (auto bounce = 0u; bounce < ctx.film->depth; bounce += pay.scattered) {
            auto q = math::max(pay.beta * math::guarded_div(1.f, math::avg(pay.mis_s)));
            if (q < 1.f) {
                if (pay.sampler.generate_1d() > q) break;
                pay.beta /= q;
            }

            pay.acc = ctx.accel(pay.ctx.r, pay.ctx.n, accel::Flags(0), {0.f, math::inf<f32>});
            // reorder
            auto div = pay.acc.divider;
            if (!div) { infra::cartesian{}([&] { miss(ctx, pay); }); break; }
            auto medium = pay.acc.pos[2] < 0.f ? div->int_medium : div->ext_medium;
            infra::cartesian{}([&]<typename... Ts>() { hit<Ts...>(ctx, pay); }, div->shape.idx, div->material, medium);

            if (pay.gamma != fv4{0.f}) {
                auto occlusion = accel::Flags(accel::Flags::hit_first | accel::Flags::skip_interface);
                auto blocker = ctx.accel(pay.shadow, fv3{0.f}, occlusion, {0.f, pay.t - 0.001f});
                if (blocker.divider) pay.gamma = fv4{0.f};
            }
            if (!ctx.integrator.idx.data<Radiative_Integrator>()->volumetric && pay.gamma != fv4{0.f}) {
                auto mis_u = math::guarded_div(1.f, math::avg(pay.mis_d + pay.mis_l));
                pay.emission += pay.gamma * mis_u;
                pay.gamma = fv4{0.f};
            }
            while (pay.gamma != fv4{0.f}) {
                pay.acc = pay.t > 0.001f ? ctx.accel(pay.shadow, fv3{0.f}, accel::Flags::only_interface, {0.f, math::inf<f32>}) : accel::Interaction{};
                auto div = pay.acc.divider;
                if (div) infra::cartesian{}([&]<typename Shape, typename Medium>() {
                    track<Shape, Medium>(ctx, pay);
                }, div->shape.idx, pay.medium); else {
                    auto mis_u = math::guarded_div(1.f, math::avg(pay.mis_d + pay.mis_l));
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

        auto Li = spectra::Stochastic_Spectrum{pay.ctx.lambda, pay.emission};
        Li.value /= s.pdf;
        if (!math::isfinite(Li.value))
            stl::abort("invalid value appears in pixel {} sample {}", px, ctx.sample_index);
        fixel = Li;
    }

    auto Radiative_Integrator::trace(infra::recorder& r, Context& ctx) const noexcept -> void {
        infra::pipenc(r, sample, ctx).dispatch({uv2{ctx.film->image.size}, 1});
    }
}
