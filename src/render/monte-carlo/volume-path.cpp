#include <metatron/render/monte-carlo/volume-path.hpp>
#include <metatron/core/math/distribution/discrete.hpp>
#include <metatron/core/math/quaternion.hpp>
#include <metatron/core/math/arithmetic.hpp>
#include <metatron/core/math/plane.hpp>
#include <metatron/core/stl/optional.hpp>

namespace mtt::monte_carlo {
    auto Volume_Path_Integrator::sample(
        Context context,
        view<accel::Acceleration> accel,
        view<emitter::Emitter> emitter,
        mut<sampler::Sampler> sampler
    ) const noexcept -> std::optional<spectra::Stochastic_Spectrum> {
        auto lambda_u = sampler->generate_1d();
        auto emission = spectra::Stochastic_Spectrum{lambda_u};
        auto beta = emission; beta = 1.f;
        auto mis_s = emission; mis_s = 1.f;
        auto mis_e = emission; mis_e = 0.f;

        auto depth = 0uz;
        auto max_depth = context.max_depth;

        auto scattered = false;
        auto crossed = true;
        auto terminated = false;

        auto p = 0.f;
        auto f = emission;
        auto bsdf = poly<bsdf::Bsdf>{};
        auto phase = poly<phase::Phase_Function>{};

        auto trace_ctx = eval::Context{};
        auto history_ctx = eval::Context{};
        trace_ctx.r = context.ray_differential.r;
        trace_ctx.spec = emission;

        auto acc_opt = std::optional<accel::Interaction>{};
        auto medium = proxy<media::Medium>{};
        auto medium_to_render = proxy<math::Transform>{};
        auto& rdiff = context.ray_differential;
        auto& ddiff = context.default_differential;
        auto& ct = context.render_to_camera;

        while (true) {
            depth += usize(scattered);
            if (terminated || depth >= max_depth) break;

            auto q = spectra::max(beta * math::guarded_div(1.f, spectra::avg(mis_s)));
            if (q < 1.f && depth > 1uz) {
                auto rr_u = sampler->generate_1d();
                if (rr_u > q) {
                    terminated = true;
                    continue;
                } else {
                    beta /= q;
                }
            }

            [&]() {
                if (!scattered) return;

                auto direct_ctx = trace_ctx;
                MTT_OPT_OR_RETURN(e_intr, emitter->sample(direct_ctx, sampler->generate_1d()));
                auto& et = *(e_intr.local_to_render);
                auto light = e_intr.light;
                auto l_ctx = et ^ direct_ctx;
                MTT_OPT_OR_RETURN(l_intr, light->sample(l_ctx, sampler->generate_2d()));

                auto e_pdf = e_intr.pdf;
                auto l_pdf = e_intr.light->pdf({l_ctx.r.o, l_intr.wi}, l_ctx.n);
                auto p_e = e_pdf * l_pdf;
                if (math::abs(p_e) < math::epsilon<f32>) return;

                l_intr.p = et | math::expand(l_intr.p, 1.f);
                l_intr.wi = math::normalize(et | math::expand(l_intr.wi, 0.f));
                direct_ctx.r.d = l_intr.wi;

                auto q = 0.f;
                auto g = spectra::Stochastic_Spectrum{};

                if (direct_ctx.n != math::Vector<f32, 3>{0.f}) {
                    auto flip_n = math::dot(-history_ctx.r.d, direct_ctx.n) < 0.f ? -1.f : 1.f;
                    auto t = math::Transform{math::Matrix<f32, 4, 4>{
                        math::Quaternion<f32>::from_rotation_between(flip_n * direct_ctx.n, {0.f, 1.f, 0.f})
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

                auto acc_opt = std::optional<accel::Interaction>{};
                auto terminated = false;
                auto crossed = true;

                auto volume = medium;
                auto direct_to_render = medium_to_render;
                auto gamma = beta * (g / p_e) / (f / p);
                auto mis_d = mis_s * q / p_e * f32(!(e_intr.light->flags() & light::Flags::delta));
                auto mis_l = mis_s;

                while (true) {
                    if (terminated || l_intr.t < 0.001f) break;

                    if (spectra::max(gamma * math::guarded_div(1.f, spectra::avg(mis_d + mis_l))) < 0.05f) {
                        auto q = 0.25f;
                        if (sampler->generate_1d() > q) {
                            gamma = 0.f;
                            terminated = true;
                            continue;
                        } else {
                            gamma /= q;
                        }
                    }

                    if (crossed) {
                        acc_opt = (*accel)(direct_ctx.r);
                        if (!acc_opt) {
                            terminated = true;
                            continue;
                        }
                        auto& acc = acc_opt.value();

                        if (!acc.intr_opt) {
                            terminated = true;
                            continue;
                        }
                        auto& intr = acc_opt->intr_opt.value();

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
                        if (!close_to_light && !is_interface) {
                            terminated = true;
                            gamma = 0.f;
                            continue;
                        } else if (close_to_light) {
                            auto st = math::Transform{math::Matrix<f32, 4, 4>{
                                math::Quaternion<f32>::from_rotation_between(ddiff.r.d, math::normalize(intr.p))
                            }};
                            auto rd = st | ddiff;

                            auto ldiff = lt ^ rd;
                            auto d_intr = intr;
                            d_intr.p = lt ^ d_intr.p;
                            d_intr.n = lt ^ d_intr.n;
                            MTT_OPT_OR_CALLBACK(tcoord, texture::grad(ldiff, d_intr), {
                                terminated = true; gamma = 0.f; continue;
                            });
                            MTT_OPT_OR_CALLBACK(mat_intr, div.material->sample(direct_ctx, tcoord), {
                                terminated = true; gamma = 0.f; continue;
                            });
                            l_intr.L = mat_intr.emission;
                        }
                    }

                    auto& acc = acc_opt.value();
                    auto& div = acc.divider;
                    auto& intr = acc.intr_opt.value();

                    auto& mt = *direct_to_render;
                    auto m_ctx = mt ^ direct_ctx;
                    MTT_OPT_OR_CALLBACK(m_intr, volume->sample(m_ctx, intr.t, sampler->generate_1d()), {
                        gamma = 0.f;
                        terminated = true;
                        break;
                    });
                    m_intr.p = mt | math::expand(m_intr.p, 1.f);
                    l_intr.t -= m_intr.t;

                    auto hit = m_intr.t >= intr.t;
                    auto spectra_pdf = hit
                    ? m_intr.transmittance
                    : m_intr.sigma_maj * m_intr.transmittance;
                    auto flight_pdf = spectra_pdf.value[0];

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

                auto mis_u = math::guarded_div(1.f, spectra::avg(mis_d + mis_l));
                emission += gamma * mis_u * l_intr.L;
            }();

            if (scattered || crossed) acc_opt = (*accel)(trace_ctx.r);
            if (!acc_opt || !acc_opt.value().intr_opt) {
                terminated = true;

                MTT_OPT_OR_CONTINUE(e_intr, emitter->sample_infinite(trace_ctx, sampler->generate_1d()));
                auto light = e_intr.light;
                auto& lt = *e_intr.local_to_render;

                auto l_ctx = lt ^ trace_ctx;
                MTT_OPT_OR_CONTINUE(l_intr, (*light.data())(l_ctx.r, l_ctx.spec));

                auto e_pdf = e_intr.pdf;
                auto l_pdf = light->pdf(l_ctx.r, l_ctx.n);
                auto p_e = e_pdf * l_pdf;

                mis_e *= math::guarded_div(p_e, p);
                auto mis_w = math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
                emission += beta * mis_w * l_intr.L;
                continue;
            }

            auto& acc = acc_opt.value();
            auto& div = acc.divider;
            auto& intr = acc.intr_opt.value();
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
            MTT_OPT_OR_CALLBACK(m_intr, medium->sample(m_ctx, intr.t, sampler->generate_1d()), {
                terminated = true;
                continue;
            });
            m_intr.p = mt | math::expand(m_intr.p, 1.f);

            auto hit = m_intr.t >= intr.t;
            auto spectra_pdf = hit
            ? m_intr.transmittance
            : m_intr.sigma_maj * m_intr.transmittance;
            auto flight_pdf = spectra_pdf.value[0];

            beta *= m_intr.transmittance / flight_pdf;
            mis_s *= spectra_pdf / flight_pdf;
            mis_e *= spectra_pdf / flight_pdf;

            if (!hit) {
                auto mis_a = math::guarded_div(1.f, spectra::avg(mis_s));
                emission += mis_a * beta * m_intr.sigma_a * m_intr.sigma_e;

                auto p_a = math::guarded_div(m_intr.sigma_a.value[0], m_intr.sigma_maj.value[0]);
                auto p_s = math::guarded_div(m_intr.sigma_s.value[0], m_intr.sigma_maj.value[0]);
                auto p_n = math::guarded_div(m_intr.sigma_n.value[0], m_intr.sigma_maj.value[0]);

                auto u = sampler->generate_1d();
                auto mode = math::Discrete_Distribution{{p_a, p_s, p_n}}.sample(u);
                if (mode == 0uz) {
                    beta *= m_intr.sigma_a / p_a;
                    terminated = true;
                } else if (mode == 1uz) {
                    phase = std::move(m_intr.phase);
                    auto pt = math::Transform{math::Matrix<f32, 4, 4>{
                        math::Quaternion<f32>::from_rotation_between(-trace_ctx.r.d, {0.f, 1.f, 0.f})
                    }};

                    auto p_ctx = pt | trace_ctx;
                    MTT_OPT_OR_CALLBACK(p_intr, phase->sample(p_ctx, sampler->generate_2d()), {
                        terminated = true;
                        continue;
                    });
                    p_intr.wi = math::normalize(pt ^ math::expand(p_intr.wi, 0.f));

                    beta *= m_intr.sigma_s / p_s * p_intr.f / p_intr.pdf;
                    mis_s *= m_intr.sigma_s / m_intr.sigma_maj / p_s;
                    mis_e = mis_s;

                    scattered = true;
                    crossed = false;
                    f = p_intr.f;
                    p = p_intr.pdf;
                    history_ctx = trace_ctx;
                    trace_ctx = {{m_intr.p, p_intr.wi}, {}, emission};
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
                auto st = math::Transform{math::Matrix<f32, 4, 4>{
                    math::Quaternion<f32>::from_rotation_between(ddiff.r.d, math::normalize(intr.p))
                }};
                rdiff = st | ddiff;
            }
            rdiff.differentiable = false;
            auto ldiff = lt ^ rdiff;
            auto l_intr = acc.intr_opt.value();
            l_intr.p = lt ^ l_intr.p;
            l_intr.n = lt ^ l_intr.n;
            MTT_OPT_OR_BREAK(tcoord, texture::grad(ldiff, l_intr));
            MTT_OPT_OR_BREAK(mat_intr, div->material->sample(trace_ctx, tcoord));

            [&]() {
                if (spectra::max(mat_intr.emission) < math::epsilon<f32>) return;
                auto s_pdf = div->shape->pdf(trace_ctx.r, trace_ctx.n, acc.primitive);
                auto e_pdf = 1.f; // TODO: need better way to fetch area light pdf on GPU
                auto p_e = e_pdf * s_pdf;

                mis_e *= math::guarded_div(p_e, p);
                auto mis_w = math::guarded_div(1.f, spectra::avg(mis_s + mis_e));
                emission += mis_w * beta * mat_intr.emission;
            }();

            auto tbn = math::transpose(math::Matrix<f32, 3, 3>{intr.tn, intr.bn, intr.n});
            intr.n = tbn | mat_intr.normal;

            if (mat_intr.degraded && !spectra::coherent(emission)) {
                spectra::degrade(emission); spectra::degrade(beta);
                spectra::degrade(mis_s); spectra::degrade(mis_e);
                trace_ctx.spec = emission;
            }

            bsdf = std::move(mat_intr.bsdf);
            auto bt = math::Transform{math::Matrix<f32, 4, 4>{
                math::Quaternion<f32>::from_rotation_between(intr.n, {0.f, 1.f, 0.f})
            }};
            auto uc = sampler->generate_1d();
            auto u = sampler->generate_2d();

            auto b_ctx = bt | trace_ctx;
            MTT_OPT_OR_CALLBACK(b_intr, bsdf->sample(b_ctx, {uc, u[0], u[1]}), {
                terminated = true;
                continue;
            });

            if (b_ctx.r.d == b_intr.wi) {
                scattered = false;
                crossed = true;
                history_ctx = trace_ctx;
                trace_ctx.r.o = intr.p - 0.001f * intr.n;
                continue;
            }

            scattered = true;
            crossed = b_intr.wi[1] < 0.f;

            auto trace_n = (crossed ? -1.f : 1.f) * intr.n;
            auto trace_p = intr.p + 0.001f * trace_n;
            b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));
            b_intr.f *= math::abs(math::dot(b_intr.wi, trace_n));

            history_ctx = trace_ctx;
            trace_ctx = {{trace_p, b_intr.wi}, trace_n, emission};
            beta *= b_intr.f / b_intr.pdf;
            mis_e = mis_s;
            f = b_intr.f;
            p = b_intr.pdf;
        }

        if (math::isnan(emission.value) || math::isinf(emission.value)) return {};
        return emission;
    }
}
