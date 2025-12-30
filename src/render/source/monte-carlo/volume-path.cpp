#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/plane.hpp>

namespace mtt::monte_carlo {
    Volume_Path_Integrator::Volume_Path_Integrator(cref<Descriptor>) noexcept {}

    auto Volume_Path_Integrator::sample(
        ref<Context> ctx
    ) const noexcept -> opt<spectra::Stochastic_Spectrum> {
        auto emission = fv4{0.f};
        auto beta = fv4{1.f};
        auto mis_s = fv4{1.f};
        auto mis_e = fv4{0.f};

        auto depth = 0u;
        auto scattered = false;
        auto crossed = true;
        auto specular = false;

        auto p = 0.f;
        auto f = fv4{0.f};
        auto bsdf = obj<bsdf::Bsdf>{};
        auto phase = obj<phase::Phase_Function>{};

        auto trace_ctx = math::Context{};
        auto history_ctx = math::Context{};
        trace_ctx.r = ctx.ray_differential.r;
        trace_ctx.lambda = ctx.lambda;

        auto acc_opt = opt<accel::Interaction>{};
        auto medium = tag<media::Medium>{};
        auto medium_to_render = tag<math::Transform>{};
        auto& rdiff = ctx.ray_differential;
        auto& ddiff = ctx.default_differential;
        auto& ct = ctx.render_to_camera;

        auto direct_lighting = [&]() {
            if (!scattered || specular) return;

            auto direct_ctx = trace_ctx;
            MTT_OPT_OR_RETURN(e_intr, ctx.emitter->sample(direct_ctx, ctx.sampler->generate_1d()));
            auto& et = *(e_intr.local_to_render);
            auto light = e_intr.light;
            auto l_ctx = et ^ direct_ctx;
            MTT_OPT_OR_RETURN(l_intr, light->sample(l_ctx, ctx.sampler->generate_2d()));

            auto e_pdf = e_intr.pdf;
            auto l_pdf = l_intr.pdf;
            auto p_e = e_pdf * l_pdf;
            if (math::abs(p_e) < math::epsilon<f32>) return;

            l_intr.p = et | math::expand(l_intr.p, 1.f);
            l_intr.wi = math::normalize(et | math::expand(l_intr.wi, 0.f));
            direct_ctx.r.d = l_intr.wi;

            auto q = 0.f;
            auto g = fv4{0.f};

            if (direct_ctx.n != fv3{0.f}) {
                auto flip_n = math::dot(-history_ctx.r.d, direct_ctx.n) < 0.f ? -1.f : 1.f;
                auto t = math::Transform{fm44{
                    fq::from_rotation_between(flip_n * direct_ctx.n, {0.f, 1.f, 0.f})
                }};
                auto wo = math::normalize(t | math::expand(history_ctx.r.d, 0.f));
                auto wi = math::normalize(t | math::expand(l_intr.wi, 0.f));
                MTT_OPT_OR_RETURN(b_intr, (*bsdf)(wo, wi));
                g = b_intr.f * math::abs(math::dot(l_intr.wi, direct_ctx.n));
                q = b_intr.pdf;
            } else {
                MTT_OPT_OR_RETURN(p_intr, (*phase)(history_ctx.r.d, l_intr.wi));
                g = p_intr.f;
                q = p_intr.pdf;
            }

            auto acc_opt = opt<accel::Interaction>{};
            auto crossed = true;

            auto volume = medium;
            auto direct_to_render = medium_to_render;
            auto gamma = beta * (g / p_e) / (f / p);
            auto mis_d = mis_s * q / p_e * f32(!(e_intr.light->flags() & light::Flags::delta));
            auto mis_l = mis_s;

            while (true) {
                if (l_intr.t < 0.001f) break;

                if (math::max(gamma * math::guarded_div(1.f, math::avg(mis_d + mis_l))) < 0.05f) {
                    auto q = 0.25f;
                    if (ctx.sampler->generate_1d() > q) {
                        gamma = fv4{0.f}; break;
                    } else gamma /= q;
                }

                if (crossed) {
                    acc_opt = (*ctx.accel)(direct_ctx.r, direct_ctx.n);
                    if (!acc_opt || !acc_opt->intr_opt) break;
                    auto& acc = *acc_opt;
                    auto& intr = *acc.intr_opt;
                    auto& div = *acc.divider;
                    auto& lt = *div.local_to_render;

                    intr.p = lt | math::expand(intr.p, 1.f);
                    intr.n = math::normalize(lt | intr.n);
                    direct_ctx.inside = math::dot(-direct_ctx.r.d, intr.n) < 0.f;
                    volume = direct_ctx.inside ? div.int_medium : div.ext_medium;
                    direct_to_render = direct_ctx.inside ? div.int_to_render : div.ext_to_render;
                    intr.n *= direct_ctx.inside ? -1.f : 1.f;

                    auto close_to_light = math::length(intr.p - l_intr.p) < 0.001f;
                    auto is_interface = div.material->flags() & material::Flags::interface;
                    auto is_emissive = div.material->flags() & material::Flags::emissive;
                    if (!is_interface && (!is_emissive || !close_to_light)) {
                        gamma = fv4{0.f}; break;
                    } else if (close_to_light) {
                        auto st = math::Transform{fm44{
                            fq::from_rotation_between(ddiff.r.d, math::normalize(intr.p))
                        }};
                        auto rd = st | ddiff;

                        auto ldiff = lt ^ rd;
                        auto d_intr = intr;
                        d_intr.p = lt ^ d_intr.p;
                        d_intr.n = lt ^ d_intr.n;
                        MTT_OPT_OR_CALLBACK(tcoord, texture::grad(ldiff, d_intr), {
                            gamma = fv4{0.f}; break;
                        });
                        MTT_OPT_OR_CALLBACK(mat_intr, div.material->sample(direct_ctx, tcoord), {
                            gamma = fv4{0.f}; break;
                        });
                        l_intr.L = mat_intr.emission;
                    }
                }

                auto& acc = *acc_opt;
                auto& intr = *acc.intr_opt;
                auto& div = acc.divider;

                auto& mt = *direct_to_render;
                auto m_ctx = mt ^ direct_ctx;
                MTT_OPT_OR_CALLBACK(m_intr, volume->sample(m_ctx, intr.t, ctx.sampler->generate_1d()), {
                    gamma = fv4{0.f}; break;
                });
                m_intr.p = mt | math::expand(m_intr.p, 1.f);
                l_intr.t -= m_intr.t;

                auto hit = m_intr.t >= intr.t;
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
                    direct_ctx.r.o = intr.p - 0.001f * intr.n;
                    crossed = true;
                }
                continue;
            }

            auto mis_u = math::guarded_div(1.f, math::avg(mis_d + mis_l));
            emission += gamma * mis_u * l_intr.L;
        };

        while (true) {
            depth += usize(scattered);
            if (depth >= ctx.max_depth) break;

            auto q = math::max(beta * math::guarded_div(1.f, math::avg(mis_s)));
            if (q < 1.f && depth > 1u) {
                auto rr_u = ctx.sampler->generate_1d();
                if (rr_u > q) break;
                else beta /= q;
            }

            direct_lighting();
            if (scattered || crossed) acc_opt = (*ctx.accel)(trace_ctx.r, trace_ctx.n);
            if (!acc_opt || !acc_opt->intr_opt) {
                MTT_OPT_OR_BREAK(e_intr, ctx.emitter->sample_infinite(trace_ctx, ctx.sampler->generate_1d()));
                auto light = e_intr.light;
                auto& lt = *e_intr.local_to_render;

                auto l_ctx = lt ^ trace_ctx;
                MTT_OPT_OR_BREAK(l_intr, (*light.data())(l_ctx.r, l_ctx.lambda));

                mis_e *= specular ? 0.f : math::guarded_div(e_intr.pdf * l_intr.pdf, p);
                auto mis_w = math::guarded_div(1.f, math::avg(mis_s + mis_e));
                emission += beta * mis_w * l_intr.L;
                break;
            }

            auto& acc = *acc_opt;
            auto& intr = *acc.intr_opt;
            auto& div = acc.divider;
            auto& lt = *div->local_to_render;

            if (scattered || crossed) {
                intr.p = lt | math::expand(intr.p, 1.f);
                intr.n = math::normalize(lt | intr.n);
                trace_ctx.inside = math::dot(-trace_ctx.r.d, intr.n) < 0.f;
                medium = trace_ctx.inside ? div->int_medium : div->ext_medium;
                medium_to_render = trace_ctx.inside ? div->int_to_render : div->ext_to_render;

                auto flip_n = trace_ctx.inside ? -1.f : 1.f;
                intr.n *= flip_n;
                intr.tn = math::normalize(lt | math::expand(intr.tn * flip_n, 0.f));
                intr.bn = math::normalize(lt | math::expand(intr.bn * flip_n, 0.f));
            }

            auto& mt = *medium_to_render;
            auto m_ctx = mt ^ trace_ctx;
            MTT_OPT_OR_BREAK(m_intr, medium->sample(m_ctx, intr.t, ctx.sampler->generate_1d()));
            m_intr.p = mt | math::expand(m_intr.p, 1.f);

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

                auto u = ctx.sampler->generate_1d();
                auto mode = math::Discrete_Distribution<3>{{p_a, p_s, p_n}}.sample(u);
                if (mode == 0uz) {
                    beta *= m_intr.sigma_a / p_a;
                    break;
                } else if (mode == 1uz) {
                    phase = std::move(m_intr.phase);
                    auto pt = math::Transform{fm44{
                        fq::from_rotation_between(-trace_ctx.r.d, {0.f, 1.f, 0.f})
                    }};

                    auto p_ctx = pt | trace_ctx;
                    MTT_OPT_OR_BREAK(p_intr, phase->sample(p_ctx, ctx.sampler->generate_2d()));
                    p_intr.wi = math::normalize(pt ^ math::expand(p_intr.wi, 0.f));

                    beta *= m_intr.sigma_s / p_s * p_intr.f / p_intr.pdf;
                    mis_s *= m_intr.sigma_s / m_intr.sigma_maj / p_s;
                    mis_e = mis_s;

                    scattered = true;
                    specular = false;
                    crossed = false;
                    f = p_intr.f;
                    p = p_intr.pdf;
                    history_ctx = trace_ctx;
                    trace_ctx.r = {m_intr.p, p_intr.wi};
                    trace_ctx.n = {};
                } else {
                    beta *= m_intr.sigma_n / p_n;
                    mis_s *= (m_intr.sigma_n / m_intr.sigma_maj) / p_n;
                    mis_e /= p_n;

                    intr.t -= m_intr.t;
                    trace_ctx.r.o = m_intr.p;
                    scattered = false;
                    crossed = false;
                }
                continue;
            }

            if (!rdiff.differentiable) {
                auto st = math::Transform{fm44{
                    fq::from_rotation_between(ddiff.r.d, math::normalize(intr.p))
                }};
                rdiff = st | ddiff;
            }
            rdiff.differentiable = false;
            auto ldiff = lt ^ rdiff;
            auto l_intr = intr;
            l_intr.p = lt ^ l_intr.p;
            l_intr.n = lt ^ l_intr.n;
            MTT_OPT_OR_BREAK(tcoord, texture::grad(ldiff, l_intr));
            MTT_OPT_OR_BREAK(mat_intr, div->material->sample(trace_ctx, tcoord));

            if (math::max(mat_intr.emission) > math::epsilon<f32>) {
                // TODO: need correct way to fetch area light pdf on GPU, use 1.f now
                mis_e *= specular ? 0.f : math::guarded_div(intr.pdf * 1.f, p);
                auto mis_w = math::guarded_div(1.f, math::avg(mis_s + mis_e));
                emission += mis_w * beta * mat_intr.emission;
            }

            auto tbn = math::transpose(fm33{intr.tn, intr.bn, intr.n});
            intr.n = tbn | mat_intr.normal;

            if (mat_intr.degraded && !math::constant(trace_ctx.lambda)) {
                emission = fv4{emission[0]}; beta = fv4{beta[0]};
                mis_s = fv4{mis_s[0]}; mis_e = fv4{mis_e[0]};
                trace_ctx.lambda = fv4{trace_ctx.lambda[0]};
                history_ctx.lambda = trace_ctx.lambda;
            }

            bsdf = std::move(mat_intr.bsdf);
            auto bt = math::Transform{fm44{
                fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f})
            }};
            auto uc = ctx.sampler->generate_1d();
            auto u = ctx.sampler->generate_2d();

            auto b_ctx = bt | trace_ctx;
            b_ctx.r.d = math::normalize(b_ctx.r.d);
            MTT_OPT_OR_BREAK(b_intr, bsdf->sample(b_ctx, {uc, u[0], u[1]}));

            if (b_ctx.r.d == b_intr.wi) {
                scattered = false;
                crossed = true;
                history_ctx = trace_ctx;
                trace_ctx.r.o = intr.p - 0.001f * intr.n;
                continue;
            }

            scattered = true;
            specular = bsdf->flags() & bsdf::Flags::specular;
            crossed = b_intr.wi[1] < 0.f;

            auto trace_n = (crossed ? -1.f : 1.f) * intr.n;
            auto trace_p = intr.p + 0.001f * trace_n;
            b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));
            b_intr.f *= math::abs(math::dot(b_intr.wi, trace_n));

            history_ctx.r = trace_ctx.r;
            history_ctx.n = trace_ctx.n;
            trace_ctx.r = {trace_p, b_intr.wi};
            trace_ctx.n = trace_n;
            beta *= b_intr.f / b_intr.pdf;
            mis_e = mis_s;
            f = b_intr.f;
            p = b_intr.pdf;
        }

        if (math::isnan(emission) || math::isinf(emission)) return {};
        return spectra::Stochastic_Spectrum{trace_ctx.lambda, emission};
    }
}
