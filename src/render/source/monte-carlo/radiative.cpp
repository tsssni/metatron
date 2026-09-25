#include <metatron/render/monte-carlo/radiative.hpp>
#include <metatron/core/stl/thread.hpp>

namespace mtt::monte_carlo {
    Radiative_Integrator::Radiative_Integrator(cref<Descriptor>) noexcept {}

    auto Radiative_Integrator::trace(ref<Context> ctx) const noexcept -> void {
        auto ct = *math::proxy::Transform::entity("/hierarchy/camera/render");
        auto spp = ctx.film->spp;
        auto bounce = ctx.film->depth;
        auto size = uzv2{ctx.film->image.size};

        auto trace = [&](auto&& px) {
            auto sp = sampler::proxy::Sampler{ctx.sampler, {{}, px, size, ctx.sample_index, spp, 0, ctx.seed}};
            sp.start();
            auto fixel = ctx.film(ctx.filter, px, sp.generate_pixel_2d());
            MTT_OPT_OR_CALLBACK(s, photo::Camera{}.sample(
                ctx.lens, fixel.position, fixel.dxdy, sp.generate_2d()
            ), stl::abort("ray generation failed"););
            s.ray_differential = ct ^ s.ray_differential;
            auto spec = spectra::Stochastic_Spectrum{sp.generate_1d()};

            auto r = Ray{
                ctx.accel, ctx.emitter,
                sp, spec.lambda,
                s.ray_differential,
                bounce,
            };
            MTT_OPT_OR_CALLBACK(Li, sample(r),
                stl::abort("invalid value appears in pixel {} sample {}", px, ctx.sample_index);
            );
            Li.value /= s.pdf;
            fixel = Li;
        };
        stl::scheduler::sync_parallel(uzv2{size}, trace);
    }

    auto Radiative_Integrator::sample(ref<Ray> r) const noexcept -> opt<spectra::Stochastic_Spectrum> {
        auto emission = fv4{0.f};
        auto beta = fv4{1.f};
        auto mis_s = fv4{1.f};
        auto mis_e = fv4{0.f};

        auto bounce = 0u;
        auto scattered = false;
        auto crossed = true;
        auto specular = false;

        auto p = 0.f;
        auto bsdf = bsdf::Bsdf{};
        auto phase = phase::Phase_Function{};

        auto trace_ctx = math::Context{};
        trace_ctx.r = r.ray_differential.r;
        trace_ctx.lambda = r.lambda;

        auto acc_opt = opt<accel::Interaction>{};
        auto medium = media::Medium{};
        auto medium_to_render = math::proxy::Transform{};
        auto iter = media::Iterator{};
        auto& rdiff = r.ray_differential;

        auto direct_lighting = [&](cref<math::Context> ctx, auto&& propagate) {
            auto direct_ctx = ctx;
            MTT_OPT_OR_RETURN(e_intr, r.emitter.sample(direct_ctx, r.sampler.generate_1d()));
            auto et = e_intr.local_to_render;
            auto light = e_intr.light;
            auto l_ctx = et ^ direct_ctx;
            MTT_OPT_OR_RETURN(l_intr, light.sample(l_ctx, r.sampler.generate_2d()));

            auto e_pdf = e_intr.pdf;
            auto l_pdf = l_intr.pdf;
            auto p_e = e_pdf * l_pdf;
            if (math::abs(p_e) < math::epsilon<f32>) return;

            l_intr.p = et | math::expand(l_intr.p, 1.f);
            l_intr.wi = math::normalize(et | math::expand(l_intr.wi, 0.f));
            direct_ctx.r.d = l_intr.wi;

            auto q = 0.f;
            auto g = fv4{0.f};
            auto eta = fv4{1.f};

            if (ctx.n != fv3{0.f}) {
                auto flip_n = math::dot(-ctx.r.d, ctx.n) < 0.f ? -1.f : 1.f;
                auto m = fm44{fq::from_rotation_between(flip_n * ctx.n, {0.f, 1.f, 0.f})};
                auto wo = math::normalize(m | math::expand(ctx.r.d, 0.f));
                auto wi = math::normalize(m | math::expand(l_intr.wi, 0.f));
                MTT_OPT_OR_RETURN(b_intr, bsdf(wo, wi));
                g = b_intr.f * math::abs(math::dot(l_intr.wi, ctx.n));
                q = b_intr.pdf;
                eta = b_intr.eta;
                direct_ctx.n = (math::dot(l_intr.wi, ctx.n) < 0.f ? -1.f : 1.f) * ctx.n;
                direct_ctx.r.o = ctx.r.o + 0.001f * direct_ctx.n;
            } else {
                MTT_OPT_OR_RETURN(p_intr, phase(ctx.r.d, l_intr.wi));
                g = p_intr.f;
                q = p_intr.pdf;
            }
            auto diff = propagate(l_intr.wi, eta);

            auto acc_opt = opt<accel::Interaction>{};
            auto crossed = true;

            auto volume = medium;
            auto direct_to_render = medium_to_render;
            auto iter = media::Iterator{};
            auto gamma = beta * g / p_e;
            auto delta = e_intr.light.flags() & light::Flags::delta;
            auto mis_d = mis_s * q / p_e * f32(!delta);
            auto mis_l = mis_s;

            while (true) {
                if (l_intr.t < 0.001f) break;

                if (math::max(gamma * math::guarded_div(1.f, math::avg(mis_d + mis_l))) < 0.05f) {
                    auto q = 0.25f;
                    if (r.sampler.generate_1d() > q) {
                        gamma = fv4{0.f}; break;
                    } else gamma /= q;
                }

                if (crossed) {
                    acc_opt = r.accel(direct_ctx.r, direct_ctx.n);
                    if (!acc_opt || !acc_opt->intr_opt) break;
                    auto& acc = *acc_opt;
                    auto& intr = *acc.intr_opt;
                    auto& div = *acc.divider;
                    auto lt = div.local_to_render;

                    intr.p = lt | math::expand(intr.p, 1.f);
                    intr.n = math::normalize(lt | intr.n);
                    direct_ctx.inside = math::dot(-direct_ctx.r.d, intr.n) < 0.f;
                    volume = direct_ctx.inside ? div.int_medium : div.ext_medium;
                    direct_to_render = direct_ctx.inside ? div.int_to_render : div.ext_to_render;
                    iter = volume.begin(direct_to_render ^ direct_ctx, math::min(intr.t, l_intr.t));
                    intr.n *= direct_ctx.inside ? -1.f : 1.f;

                    auto light_in_medium = l_intr.t < intr.t - 0.001f;
                    auto close_to_light = math::length(intr.p - l_intr.p) < 0.001f;
                    auto is_interface = div.material.flags() & material::Flags::interface;
                    auto is_emissive = div.material.flags() & material::Flags::emissive;
                    if (!light_in_medium && !is_interface && (!is_emissive || !close_to_light)) return;
                    else if (close_to_light) {
                        auto ldiff = lt ^ diff;
                        auto d_intr = intr;
                        d_intr.p = math::shrink(lt ^ math::expand(d_intr.p, 1.f));
                        d_intr.n = lt ^ d_intr.n;
                        MTT_OPT_OR_RETURN(tcoord, texture::grad(ldiff, d_intr));
                        MTT_OPT_OR_RETURN(mat_intr, div.material.sample(direct_ctx, tcoord));
                        l_intr.L = mat_intr.emission;
                    }
                }

                auto& acc = *acc_opt;
                auto& intr = *acc.intr_opt;
                auto& div = acc.divider;

                MTT_OPT_OR_RETURN(m_intr, iter.march(r.sampler.generate_1d()));

                auto hit = m_intr.t >= math::min(intr.t, l_intr.t);
                l_intr.t -= m_intr.t;
                auto spectra_pdf = hit
                ? m_intr.transmittance
                : m_intr.sigma_maj * m_intr.transmittance;
                auto flight_pdf = spectra_pdf[0];

                gamma *= m_intr.transmittance / flight_pdf;
                mis_d *= spectra_pdf / flight_pdf;
                mis_l *= spectra_pdf / flight_pdf;

                if (!hit) {
                    gamma *= m_intr.sigma_n;
                    mis_d *= m_intr.sigma_n / m_intr.sigma_maj;
                    intr.t -= m_intr.t;
                    direct_ctx.r.o = m_intr.p;
                    crossed = false;
                } else {
                    auto mt = direct_to_render;
                    m_intr.p = mt | math::expand(m_intr.p, 1.f);
                    direct_ctx.r.o = intr.p - 0.001f * intr.n;
                    crossed = true;
                }
                continue;
            }

            auto mis_u = math::guarded_div(1.f, math::avg(mis_d + mis_l));
            emission += gamma * mis_u * l_intr.L;
        };

        while (true) {
            bounce += usize(scattered);

            auto q = math::max(beta * math::guarded_div(1.f, math::avg(mis_s)));
            if (q < 1.f) {
                auto rru = r.sampler.generate_1d();
                if (rru > q) break;
                else beta /= q;
            }

            if (scattered || crossed) acc_opt = r.accel(trace_ctx.r, trace_ctx.n);
            if (!acc_opt || !acc_opt->intr_opt) {
                MTT_OPT_OR_BREAK(e_intr, r.emitter.sample_infinite(trace_ctx, r.sampler.generate_1d()));
                auto light = e_intr.light;
                auto lt = e_intr.local_to_render;

                auto l_ctx = lt ^ trace_ctx;
                MTT_OPT_OR_BREAK(l_intr, light(l_ctx.r, l_ctx.lambda));

                mis_e *= specular ? 0.f : math::guarded_div(e_intr.pdf * l_intr.pdf, p);
                auto mis_w = math::guarded_div(1.f, math::avg(mis_s + mis_e));
                emission += beta * mis_w * l_intr.L;
                break;
            }

            auto& acc = *acc_opt;
            auto& intr = *acc.intr_opt;
            auto& div = acc.divider;
            auto lt = div->local_to_render;

            if (scattered || crossed) {
                intr.p = lt | math::expand(intr.p, 1.f);
                intr.n = math::normalize(lt | intr.n);
                trace_ctx.inside = math::dot(-trace_ctx.r.d, intr.n) < 0.f;
                medium = trace_ctx.inside ? div->int_medium : div->ext_medium;
                medium_to_render = trace_ctx.inside ? div->int_to_render : div->ext_to_render;
                iter = medium.begin(medium_to_render ^ trace_ctx, intr.t);

                auto flip_n = trace_ctx.inside ? -1.f : 1.f;
                intr.n *= flip_n; intr.dndu *= flip_n; intr.dndv *= flip_n;
                intr.tn = math::normalize(lt | math::expand(intr.tn * flip_n, 0.f));
                intr.bn = math::normalize(lt | math::expand(intr.bn * flip_n, 0.f));
            }

            MTT_OPT_OR_BREAK(m_intr, iter.march(r.sampler.generate_1d()));

            auto hit = m_intr.t >= intr.t;
            auto spectra_pdf = hit
            ? m_intr.transmittance
            : m_intr.sigma_maj * m_intr.transmittance;
            auto flight_pdf = spectra_pdf[0];

            beta *= m_intr.transmittance / flight_pdf;
            mis_s *= spectra_pdf / flight_pdf;
            mis_e *= spectra_pdf / flight_pdf;

            if (!hit) {
                auto mis_a = math::guarded_div(1.f, math::avg(mis_s));
                emission += mis_a * beta * m_intr.sigma_a * m_intr.sigma_e;

                auto p_a = math::guarded_div(m_intr.sigma_a[0], m_intr.sigma_maj[0]);
                auto p_s = math::guarded_div(m_intr.sigma_s[0], m_intr.sigma_maj[0]);
                auto p_n = math::guarded_div(m_intr.sigma_n[0], m_intr.sigma_maj[0]);

                auto u = r.sampler.generate_1d();
                auto mode = math::Discrete_Distribution<3>{{p_a, p_s, p_n}}.sample(u);
                if (mode == 0uz) break;
                else if (mode == 1uz) {
                    if (bounce >= r.max_bounce) break;
                    auto mt = medium_to_render;
                    m_intr.p = mt | math::expand(m_intr.p, 1.f);

                    auto redirect = [&](cref<fv3> d) {
                        auto translate = [](cref<fv3> t) { auto m = fm44{1.f}; for (auto i = 0; i < 3; i++) m[i][3] = t[i]; return m; };
                        auto rot = fm44{fq::from_rotation_between(math::normalize(rdiff.r.d), d)};
                        auto st = math::Transform{translate(m_intr.p) | rot | translate(-rdiff.r.o)};
                        return st | rdiff;
                    };

                    phase = std::move(m_intr.phase);
                    beta *= m_intr.sigma_s / p_s;
                    mis_s *= m_intr.sigma_s / m_intr.sigma_maj / p_s;

                    auto d_ctx = trace_ctx;
                    d_ctx.r.o = m_intr.p;
                    d_ctx.n = {};
                    direct_lighting(d_ctx, [&](cref<fv3> wi, cref<fv4>) { return redirect(wi); });

                    auto pt = math::Transform{};
                    pt.transform = fm44{fq::from_rotation_between(-trace_ctx.r.d, {0.f, 1.f, 0.f})};
                    pt.inv_transform = math::transpose(pt.transform);
                    auto p_ctx = pt | trace_ctx;
                    MTT_OPT_OR_BREAK(p_intr, phase.sample(p_ctx, r.sampler.generate_2d()));
                    p_intr.wi = math::normalize(pt ^ math::expand(p_intr.wi, 0.f));

                    beta *= p_intr.f / p_intr.pdf;
                    mis_e = mis_s;

                    scattered = true;
                    specular = false;
                    crossed = false;
                    p = p_intr.pdf;
                    trace_ctx.r = {m_intr.p, p_intr.wi};
                    trace_ctx.n = {};
                    rdiff = redirect(p_intr.wi);
                } else {
                    beta *= m_intr.sigma_n / p_n;
                    mis_s *= (m_intr.sigma_n / m_intr.sigma_maj) / p_n;
                    mis_e /= p_n;
                    scattered = false;
                    crossed = false;
                }
                continue;
            }

            auto ldiff = lt ^ rdiff;
            auto l_intr = intr;
            l_intr.p = math::shrink(lt ^ math::expand(l_intr.p, 1.f));
            l_intr.n = lt ^ l_intr.n;
            MTT_OPT_OR_BREAK(tcoord, texture::grad(ldiff, l_intr));
            MTT_OPT_OR_BREAK(mat_intr, div->material.sample(trace_ctx, tcoord));

            if (math::max(mat_intr.emission) > math::epsilon<f32>) {
                // TODO: need correct way to fetch area light pdf on GPU, use 1.f now
                mis_e *= specular ? 0.f : math::guarded_div(intr.pdf * 1.f, p);
                auto mis_w = math::guarded_div(1.f, math::avg(mis_s + mis_e));
                emission += mis_w * beta * mat_intr.emission;
            }

            auto tbn = math::transpose(fm33{intr.tn, intr.bn, intr.n});
            intr.n = tbn | mat_intr.normal;

            bsdf = std::move(mat_intr.bsdf);
            auto flags = bsdf.flags();
            auto interface = bool(flags & bsdf::Flags::interface);
            if (!interface && bounce >= r.max_bounce) break;
            if (!interface && !(flags & bsdf::Flags::specular)) {
                auto d_ctx = trace_ctx;
                d_ctx.r.o = intr.p;
                d_ctx.n = intr.n;
                direct_lighting(d_ctx, [&](cref<fv3> wi, cref<fv4> eta) {
                    auto l_wi = math::normalize(math::shrink(lt ^ math::expand(wi, 0.f)));
                    return lt | texture::propagate(ldiff, l_intr, tcoord, l_wi, eta);
                });
            }

            auto bt = math::Transform{};
            bt.transform = fm44{fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
            bt.inv_transform = math::transpose(bt.transform);
            auto cu = r.sampler.generate_1d();
            auto du = r.sampler.generate_2d();

            auto b_ctx = bt | trace_ctx;
            b_ctx.r.d = math::normalize(b_ctx.r.d);
            MTT_OPT_OR_BREAK(b_intr, bsdf.sample(b_ctx, {cu, du[0], du[1]}));

            if (b_ctx.r.d == b_intr.wi) {
                scattered = false;
                crossed = true;
                trace_ctx.r.o = intr.p - 0.001f * intr.n;
                continue;
            }

            scattered = true;
            specular = flags & bsdf::Flags::specular;
            crossed = b_intr.wi[1] < 0.f;

            auto trace_n = (crossed ? -1.f : 1.f) * intr.n;
            auto trace_p = intr.p + 0.001f * trace_n;
            b_intr.f *= math::abs(math::unit_to_cos_theta(b_intr.wi));
            b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));

            auto l_wi = math::normalize(math::shrink(lt ^ math::expand(b_intr.wi, 0.f)));
            rdiff = lt | texture::propagate(ldiff, l_intr, tcoord, l_wi, b_intr.eta);

            trace_ctx.r = {trace_p, b_intr.wi};
            trace_ctx.n = trace_n;
            beta *= b_intr.f / b_intr.pdf;
            mis_e = mis_s;
            p = b_intr.pdf;

            if (mat_intr.degraded && crossed && !math::constant(trace_ctx.lambda)) {
                emission = fv4{emission[0]}; beta = fv4{beta[0]};
                mis_s = fv4{mis_s[0]}; mis_e = fv4{mis_e[0]};
                trace_ctx.lambda = fv4{trace_ctx.lambda[0]};
            }
        }

        if (!math::isfinite(emission)) return {};
        return spectra::Stochastic_Spectrum{trace_ctx.lambda, emission};
    }
}
