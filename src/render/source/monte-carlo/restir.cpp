#include <metatron/render/monte-carlo/restir.hpp>

namespace mtt::monte_carlo {
    Restir_Integrator::Restir_Integrator(cref<Descriptor>) noexcept {}

    auto Restir_Integrator::sample(
        ref<Context> ctx
    ) const noexcept -> opt<spectra::Stochastic_Spectrum> {
        auto emission = fv4{0.f};
        auto beta = fv4{1.f};

        auto depth = 0u;
        auto scattered = false;
        auto specular = false;

        auto p = 0.f;
        auto f = fv4{0.f};
        auto bsdf = bsdf::Bsdf{};

        auto trace_ctx = math::Context{};
        auto history_ctx = math::Context{};
        trace_ctx.r = ctx.ray_differential.r;
        trace_ctx.lambda = ctx.lambda;

        auto acc_opt = opt<accel::Interaction>{};
        auto& rdiff = ctx.ray_differential;
        auto& ddiff = ctx.default_differential;
        auto& ct = ctx.render_to_camera;

        auto direct_lighting = [&]() {
            if (!scattered || specular) return;

            auto direct_ctx = trace_ctx;
            auto eu = ctx.sampler.generate_1d();
            auto lu = ctx.sampler.generate_2d();
            MTT_OPT_OR_RETURN(e_intr, ctx.emitter.sample(direct_ctx, eu));
            auto et = e_intr.local_to_render;
            auto light = e_intr.light;
            auto l_ctx = et ^ direct_ctx;
            MTT_OPT_OR_RETURN(l_intr, light.sample(l_ctx, lu));

            auto e_pdf = e_intr.pdf;
            auto l_pdf = l_intr.pdf;
            auto p_e = e_pdf * l_pdf;
            if (math::abs(p_e) < math::epsilon<f32>) return;

            l_intr.p = et | math::expand(l_intr.p, 1.f);
            l_intr.wi = math::normalize(et | math::expand(l_intr.wi, 0.f));
            direct_ctx.r.d = l_intr.wi;

            auto flip_n = math::dot(-history_ctx.r.d, direct_ctx.n) < 0.f ? -1.f : 1.f;
            auto m = fm44{fq::from_rotation_between(flip_n * direct_ctx.n, {0.f, 1.f, 0.f})};
            auto wo = math::normalize(m | math::expand(history_ctx.r.d, 0.f));
            auto wi = math::normalize(m | math::expand(l_intr.wi, 0.f));
            MTT_OPT_OR_RETURN(b_intr, bsdf(wo, wi));
            auto g = b_intr.f * math::abs(math::dot(l_intr.wi, direct_ctx.n));
            auto q = b_intr.pdf;
            auto gamma = beta * (g / p_e) / (f / p);

            auto acc_opt = opt<accel::Interaction>{};
            auto mis_d = q / p_e * f32(!(e_intr.light.flags() & light::Flags::delta));
            auto mis_l = fv4{1.f};
            auto mis_u = math::guarded_div(1.f, math::avg(mis_d + mis_l));
            acc_opt = ctx.accel(direct_ctx.r, direct_ctx.n);
            if (!acc_opt || !acc_opt->intr_opt) {
                emission += mis_u * gamma * l_intr.L;
                return;
            }

            auto& acc = *acc_opt;
            auto& intr = *acc.intr_opt;
            auto& div = *acc.divider;
            auto lt = div.local_to_render;

            intr.p = lt | math::expand(intr.p, 1.f);
            intr.n = math::normalize(lt | intr.n);
            direct_ctx.inside = math::dot(-direct_ctx.r.d, intr.n) < 0.f;
            intr.n *= direct_ctx.inside ? -1.f : 1.f;

            auto light_before_surface = l_intr.t < intr.t - 0.001f;
            auto close_to_light = math::length(intr.p - l_intr.p) < 0.001f;
            auto is_emissive = div.material.flags() & material::Flags::emissive;
            if (!light_before_surface && (!is_emissive || !close_to_light)) return;
            else if (!light_before_surface) {
                auto st = math::Transform{};
                st.transform = fm44{fq::from_rotation_between(ddiff.r.d, math::normalize(intr.p))};
                st.inv_transform = math::transpose(st.transform);
                auto rd = st | ddiff;

                auto ldiff = lt ^ rd;
                auto d_intr = intr;
                d_intr.p = lt ^ d_intr.p;
                d_intr.n = lt ^ d_intr.n;
                MTT_OPT_OR_RETURN(tcoord, texture::grad(ldiff, d_intr));
                MTT_OPT_OR_RETURN(mat_intr, div.material.sample(direct_ctx, tcoord));
                l_intr.L = mat_intr.emission;
            }
            emission += mis_u * gamma * l_intr.L;
        };

        while (true) {
            depth += usize(scattered);
            if (depth >= ctx.max_depth) break;

            auto q = math::max(beta);
            if (q < 1.f) {
                auto rr_u = ctx.sampler.generate_1d();
                if (rr_u > q) break;
                else beta /= q;
            }

            direct_lighting();
            acc_opt = ctx.accel(trace_ctx.r, trace_ctx.n);
            if (!acc_opt || !acc_opt->intr_opt) {
                MTT_OPT_OR_BREAK(e_intr, ctx.emitter.sample_infinite(trace_ctx, ctx.sampler.generate_1d()));
                auto light = e_intr.light;
                auto lt = e_intr.local_to_render;

                auto l_ctx = lt ^ trace_ctx;
                MTT_OPT_OR_BREAK(l_intr, light(l_ctx.r, l_ctx.lambda));

                auto mis_e = fv4{specular ? 0.f : math::guarded_div(e_intr.pdf * l_intr.pdf, p)};
                auto mis_w = math::guarded_div(1.f, math::avg(1.f + mis_e));
                emission += beta * mis_w * l_intr.L;
                break;
            }

            auto& acc = *acc_opt;
            auto& intr = *acc.intr_opt;
            auto& div = acc.divider;
            auto lt = div->local_to_render;

            intr.p = lt | math::expand(intr.p, 1.f);
            intr.n = math::normalize(lt | intr.n);
            trace_ctx.inside = math::dot(-trace_ctx.r.d, intr.n) < 0.f;

            auto flip_n = trace_ctx.inside ? -1.f : 1.f;
            intr.n *= flip_n;
            intr.tn = math::normalize(lt | math::expand(intr.tn * flip_n, 0.f));
            intr.bn = math::normalize(lt | math::expand(intr.bn * flip_n, 0.f));

            if (!rdiff.differentiable) {
                auto st = math::Transform{};
                st.transform = fm44{fq::from_rotation_between(ddiff.r.d, math::normalize(intr.p))};
                st.inv_transform = math::transpose(st.transform);
                rdiff = st | ddiff;
            }
            rdiff.differentiable = false;
            auto ldiff = lt ^ rdiff;
            auto l_intr = intr;
            l_intr.p = lt ^ l_intr.p;
            l_intr.n = lt ^ l_intr.n;
            MTT_OPT_OR_BREAK(tcoord, texture::grad(ldiff, l_intr));
            MTT_OPT_OR_BREAK(mat_intr, div->material.sample(trace_ctx, tcoord));

            if (math::max(mat_intr.emission) > math::epsilon<f32>) {
                // TODO: need correct way to fetch area light pdf on GPU, use 1.f now
                auto mis_e = fv4{specular ? 0.f : math::guarded_div(intr.pdf * 1.f, p)};
                auto mis_w = math::guarded_div(1.f, math::avg(1.f + mis_e));
                emission += mis_w * beta * mat_intr.emission;
            }

            auto tbn = math::transpose(fm33{intr.tn, intr.bn, intr.n});
            intr.n = tbn | mat_intr.normal;

            if (mat_intr.degraded && !math::constant(trace_ctx.lambda)) {
                emission = fv4{emission[0]}; beta = fv4{beta[0]};
                trace_ctx.lambda = fv4{trace_ctx.lambda[0]};
                history_ctx.lambda = trace_ctx.lambda;
            }

            bsdf = std::move(mat_intr.bsdf);
            auto bt = math::Transform{};
            bt.transform = fm44{fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f})};
            bt.inv_transform = math::transpose(bt.transform);
            auto uc = ctx.sampler.generate_1d();
            auto u = ctx.sampler.generate_2d();

            auto b_ctx = bt | trace_ctx;
            b_ctx.r.d = math::normalize(b_ctx.r.d);
            MTT_OPT_OR_BREAK(b_intr, bsdf.sample(b_ctx, {uc, u[0], u[1]}));

            scattered = true;
            specular = bsdf.flags() & bsdf::Flags::specular;
            auto trace_n = (b_intr.wi[1] < 0.f ? -1.f : 1.f) * intr.n;
            auto trace_p = intr.p + 0.001f * trace_n;
            b_intr.wi = math::normalize(bt ^ math::expand(b_intr.wi, 0.f));
            b_intr.f *= math::abs(math::dot(b_intr.wi, trace_n));

            history_ctx.r = trace_ctx.r;
            history_ctx.n = trace_ctx.n;
            trace_ctx.r = {trace_p, b_intr.wi};
            trace_ctx.n = trace_n;
            beta *= b_intr.f / b_intr.pdf;
            f = b_intr.f;
            p = b_intr.pdf;
        }

        if (!math::isfinite(emission)) return {};
        return spectra::Stochastic_Spectrum{trace_ctx.lambda, emission};
    }
}
