implementing monte_carlo;

namespace mtt::monte_carlo {
    public struct Restir_Integrator {
        u32 padding = 0u;

        public func sample(Context ctx) -> Optional<spectra::Stochastic_Spectrum> {
            var emission = fv4(0.f);
            var beta = fv4(1.f);

            var depth = 0u;
            var scattered = false;
            var specular = false;

            var p = 0.f;
            var f = fv4(0.f);
            var bsdf: bsdf::Bsdf;

            var trace_ctx: math::Context;
            var history_ctx: math::Context;
            trace_ctx.r = ctx.ray_differential.r;
            trace_ctx.lambda = ctx.lambda;

            var acc_opt: Optional<accel::Interaction>;
            var rdiff = ctx.ray_differential;
            var ddiff = ctx.default_differential;
            let ct = ctx.render_to_camera;

            while (true) {
                depth += u32(scattered);
                if (depth >= ctx.max_depth) break;

                let q = math::max(beta);
                if (q < 1.f) {
                    let rr_u = ctx.sampler.generate_1d();
                    if (rr_u > q) break;
                    else beta /= q;
                }

                do {
                    if (!scattered || specular) break;
                    var direct_ctx = trace_ctx;

                    let e_opt = ctx.emitter.sample(direct_ctx, ctx.sampler.generate_1d());
                    if (e_opt == none) break;
                    let e_intr = e_opt.value;
                    let et = *e_intr.local_to_render.get();

                    let light = e_intr.light;
                    let l_ctx = et / direct_ctx;
                    let l_opt = light.sample(l_ctx, ctx.sampler.generate_2d());
                    if (l_opt == none) break;
                    var l_intr = l_opt.value;

                    let e_pdf = e_intr.pdf;
                    let l_pdf = l_intr.pdf;
                    let p_e = e_pdf * l_pdf;
                    if (abs(p_e) < math::epsilon) break;

                    l_intr.p = (et * fv4(l_intr.p, 1.f)).xyz;
                    l_intr.wi = normalize((et * fv4(l_intr.wi, 0.f)).xyz);
                    direct_ctx.r.d = l_intr.wi;

                    let flip_n = dot(-history_ctx.r.d, direct_ctx.n) < 0.f ? -1.f : 1.f;
                    let m = fq::from_rotation_between(flip_n * direct_ctx.n, {0.f, 1.f, 0.f}).transform();
                    let wo = normalize(mul(m, fv4(history_ctx.r.d, 0.f)).xyz);
                    let wi = normalize(mul(m, fv4(l_intr.wi, 0.f)).xyz);
                    let b_opt = bsdf(wo, wi);
                    if (b_opt == none) break;
                    let b_intr = b_opt.value;
                    let g = b_intr.f * abs(dot(l_intr.wi, direct_ctx.n));
                    let q = b_intr.pdf;
                    let gamma = beta * (g / p_e) / (f / p);

                    var acc_opt: Optional<accel::Interaction>;
                    let mis_d = q / p_e * f32(!bool(e_intr.light.flags() & mtt::light::Flags::delta));
                    let mis_l = fv4(1.f);
                    let mis_u = math::guarded_div(1.f, math::avg(mis_d + mis_l));
                    acc_opt = ctx.accel(direct_ctx.r, direct_ctx.n);
                    if (!acc_opt.hasValue || !acc_opt.value.valid) {
                        emission += mis_u * gamma * l_intr.L;
                        break;
                    }

                    let acc = acc_opt.value;
                    var intr = acc.intr;
                    let div = acc.divider.get();
                    let lt = *div.local_to_render.get();

                    intr.p = (lt * fv4(intr.p, 1.f)).xyz;
                    intr.n = normalize(lt * intr.n);
                    direct_ctx.inside = dot(-direct_ctx.r.d, intr.n) < 0.f;
                    intr.n *= direct_ctx.inside ? -1.f : 1.f;

                    let close_to_light = length(intr.p - l_intr.p) < 0.001f;
                    let is_emissive = bool(div.material.flags() & material::Flags::emissive);
                    if (!is_emissive || !close_to_light) break;
                    else {
                        let stm = fq::from_rotation_between(ddiff.r.d, normalize(intr.p)).transform();
                        let st = math::Transform(stm, transpose(stm));
                        let rd = st * ddiff;

                        let ldiff = lt / rd;
                        var d_intr = intr;
                        d_intr.p = lt / d_intr.p;
                        d_intr.n = lt / d_intr.n;

                        let tcoord_opt = texture::grad(ldiff, d_intr);
                        if (tcoord_opt == none) break;
                        let tcoord = tcoord_opt.value;

                        let mat_opt = div.material.sample(direct_ctx, tcoord);
                        if (mat_opt == none) break;
                        let mat_intr = mat_opt.value;
                        l_intr.L = mat_intr.emission;
                    }
                    emission += mis_u * gamma * l_intr.L;
                } while (false);

                acc_opt = ctx.accel(trace_ctx.r, trace_ctx.n);
                if (!acc_opt.hasValue || !acc_opt.value.valid) {
                    let e_opt = ctx.emitter.sample_infinite(trace_ctx, ctx.sampler.generate_1d());
                    if (e_opt == none) break;
                    let e_intr = e_opt.value;

                    let light = e_intr.light;
                    let lt = *e_intr.local_to_render.get();

                    let l_ctx = lt / trace_ctx;
                    let l_opt = light(l_ctx.r, l_ctx.lambda);
                    if (l_opt == none) break;
                    let l_intr = l_opt.value;

                    let mis_e = fv4(specular ? 0.f : math::guarded_div(e_intr.pdf * l_intr.pdf, p));
                    let mis_w = math::guarded_div(1.f, math::avg(fv4(1.f) + mis_e));
                    emission += beta * mis_w * l_intr.L;
                    break;
                }

                let acc = acc_opt.value;
                var intr = acc.intr;
                let div = acc.divider.get();
                let lt = *div.local_to_render.get();

                intr.p = (lt * fv4(intr.p, 1.f)).xyz;
                intr.n = normalize(lt * intr.n);
                trace_ctx.inside = dot(-trace_ctx.r.d, intr.n) < 0.f;

                let flip_n = trace_ctx.inside ? -1.f : 1.f;
                intr.n *= flip_n;
                intr.tn = normalize((lt * fv4(intr.tn * flip_n, 0.f)).xyz);
                intr.bn = normalize((lt * fv4(intr.bn * flip_n, 0.f)).xyz);

                if (!rdiff.differentiable) {
                    let stm = fq::from_rotation_between(ddiff.r.d, normalize(intr.p)).transform();
                    let st = math::Transform(stm, transpose(stm));
                    rdiff = st * ddiff;
                }
                rdiff.differentiable = false;
                let ldiff = lt / rdiff;
                var l_intr = intr;
                l_intr.p = lt / l_intr.p;
                l_intr.n = lt / l_intr.n;
                let tcoord_opt = texture::grad(ldiff, l_intr);
                if (tcoord_opt == none) break;
                let tcoord = tcoord_opt.value;
                let mat_opt = div.material.sample(trace_ctx, tcoord);
                if (mat_opt == none) break;
                let mat_intr = mat_opt.value;

                if (math::max(mat_intr.emission) > math::epsilon) {
                    // TODO: need correct way to fetch area light pdf on GPU, use 1.f now
                    let mis_e = fv4(specular ? 0.f : math::guarded_div(intr.pdf * 1.f, p));
                    let mis_w = math::guarded_div(1.f, math::avg(fv4(1.f) + mis_e));
                    emission += mis_w * beta * mat_intr.emission;
                }

                let tbn = transpose(fm33(intr.tn, intr.bn, intr.n));
                intr.n = mul(tbn, mat_intr.normal);

                if (mat_intr.degraded && !math::constant(trace_ctx.lambda)) {
                    emission = fv4(emission[0]); beta = fv4(beta[0]);
                    trace_ctx.lambda = fv4(trace_ctx.lambda[0]);
                    history_ctx.lambda = trace_ctx.lambda;
                }

                bsdf = mat_intr.bsdf;
                let btm = fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f}).transform();
                let bt = math::Transform(btm, transpose(btm));
                let uc = ctx.sampler.generate_1d();
                let u = ctx.sampler.generate_2d();

                var b_ctx = bt * trace_ctx;
                b_ctx.r.d = normalize(b_ctx.r.d);
                let b_opt = bsdf.sample(b_ctx, {uc, u[0], u[1]});
                if (b_opt == none) break;
                var b_intr = b_opt.value;

                scattered = true;
                specular = (bsdf.flags() & mtt::bsdf::Flags::specular) != 0;
                let trace_n = (b_intr.wi[1] < 0.f ? -1.f : 1.f) * intr.n;
                let trace_p = intr.p + 0.001f * trace_n;
                b_intr.wi = normalize((bt / fv4(b_intr.wi, 0.f)).xyz);
                b_intr.f *= abs(dot(b_intr.wi, trace_n));

                history_ctx.r = trace_ctx.r;
                history_ctx.n = trace_ctx.n;
                trace_ctx.r = {trace_p, b_intr.wi};
                trace_ctx.n = trace_n;
                beta *= b_intr.f / b_intr.pdf;
                f = b_intr.f;
                p = b_intr.pdf;
            }

            if (!all(isfinite(emission))) return {};
            return spectra::Stochastic_Spectrum(trace_ctx.lambda, emission);
        }
    }
}
