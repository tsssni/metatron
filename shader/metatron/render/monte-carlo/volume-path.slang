implementing monte_carlo;

namespace mtt::monte_carlo {
    public struct Volume_Path_Integrator: Integrator {
        u32 padding = 0u;

        public func sample(Context ctx) -> Optional<spectra::Stochastic_Spectrum> {
            var emission = fv4(0.f);
            var beta = fv4(1.f);
            var mis_s = fv4(1.f);
            var mis_e = fv4(0.f);

            var depth = 0u;
            var scattered = false;
            var crossed = true;
            var specular = false;

            var p = 0.f;
            var f = fv4(0.f);
            var bsdf: bsdf::Bsdf_Union;
            var phase: phase::Phase_Function_Union;

            var trace_ctx: math::Context;
            var history_ctx: math::Context;
            trace_ctx.r = ctx.ray_differential.r;
            trace_ctx.lambda = ctx.lambda;

            var acc_opt: Optional<accel::Interaction>;
            var medium: media::Medium_Tag;
            var medium_to_render: tag<math::Transform>;
            var rdiff = ctx.ray_differential;
            var ddiff = ctx.default_differential;
            let ct = ctx.render_to_camera;

            while (true) {
                depth += u32(scattered);
                if (depth >= ctx.max_depth) break;

                let q = math::max(beta * math::guarded_div(1.f, math::avg(mis_s)));
                if (q < 1.f && depth > 1) {
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

                    var q = 0.f;
                    var g = fv4(0.f);

                    if (any(direct_ctx.n != fv3(0.f))) {
                        let flip_n = dot(-history_ctx.r.d, direct_ctx.n) < 0.f ? -1.f : 1.f;
                        let t = math::Transform(
                            fq::from_rotation_between(flip_n * direct_ctx.n, {0.f, 1.f, 0.f}).transform()
                        );
                        let wo = normalize((t * fv4(history_ctx.r.d, 0.f)).xyz);
                        let wi = normalize((t * fv4(l_intr.wi, 0.f)).xyz);
                        let b_opt = bsdf(wo, wi);
                        if (b_opt == none) break;
                        let b_intr = b_opt.value;
                        g = b_intr.f * abs(dot(l_intr.wi, direct_ctx.n));
                        q = b_intr.pdf;
                    } else {
                        let p_opt = phase(history_ctx.r.d, l_intr.wi);
                        if (p_opt== none) break;
                        let p_intr = p_opt.value;
                        g = p_intr.f;
                        q = p_intr.pdf;
                    }

                    var acc_opt: Optional<accel::Interaction>;
                    var crossed = true;

                    var volume = medium;
                    var direct_to_render = medium_to_render;
                    var gamma = beta * (g / p_e) / (f / p);
                    var mis_d = mis_s * q / p_e * f32(!bool(e_intr.light.flags() & mtt::light::Flags::delta));
                    var mis_l = mis_s;

                    while (true) {
                        if (l_intr.t < 0.001f) break;

                        if (math::max(gamma * math::guarded_div(1.f, math::avg(mis_d + mis_l))) < 0.05f) {
                            let q = 0.25f;
                            if (ctx.sampler.generate_1d() > q) {
                                gamma = fv4(0.f); break;
                            } else gamma /= q;
                        }

                        if (crossed) {
                            acc_opt = ctx.accel(direct_ctx.r, direct_ctx.n);
                            if (!acc_opt.hasValue || !acc_opt.value.intr_opt.hasValue) break;
                            let acc = acc_opt.value;
                            var intr = acc.intr_opt.value;
                            let div = acc.divider;
                            let lt = *div.local_to_render.get();

                            intr.p = (lt * fv4(intr.p, 1.f)).xyz;
                            intr.n = normalize(lt * intr.n);
                            direct_ctx.inside = dot(-direct_ctx.r.d, intr.n) < 0.f;
                            volume = direct_ctx.inside ? div.int_medium : div.ext_medium;
                            direct_to_render = direct_ctx.inside ? div.int_to_render : div.ext_to_render;
                            intr.n *= direct_ctx.inside ? -1.f : 1.f;

                            let close_to_light = length(intr.p - l_intr.p) < 0.001f;
                            let is_interface = bool(div.material.flags() & material::Flags::interface);
                            let is_emissive = bool(div.material.flags() & material::Flags::emissive);
                            if (!is_interface && (!is_emissive || !close_to_light)) {
                                gamma = fv4(0.f); break;
                            } else if (close_to_light) {
                                let st = math::Transform(
                                    fq::from_rotation_between(ddiff.r.d, normalize(intr.p)).transform()
                                );
                                let rd = st * ddiff;

                                let ldiff = lt / rd;
                                var d_intr = intr;
                                d_intr.p = lt / d_intr.p;
                                d_intr.n = lt / d_intr.n;

                                let tcoord_opt = texture::grad(ldiff, d_intr);
                                if (tcoord_opt == none) { gamma = fv4(0.f); break; };
                                let tcoord = tcoord_opt.value;

                                let mat_opt = div.material.sample(direct_ctx, tcoord);
                                if (mat_opt == none) { gamma = fv4(0.f); break; };
                                let mat_intr = mat_opt.value;
                                l_intr.L = mat_intr.emission;
                            }
                        }

                        let acc = acc_opt.value;
                        var intr = acc.intr_opt.value;
                        let div = acc.divider;

                        let mt = *direct_to_render.get();
                        let m_ctx = mt / direct_ctx;
                        let m_opt = volume.sample(m_ctx, intr.t, ctx.sampler.generate_1d());
                        if (m_opt == none) { gamma = fv4(0.f); break; };
                        var m_intr = m_opt.value;
                        m_intr.p = (mt * fv4(m_intr.p, 1.f)).xyz;
                        l_intr.t -= m_intr.t;

                        let hit = m_intr.t >= intr.t;
                        let spectra_pdf = hit
                        ? m_intr.transmittance
                        : m_intr.sigma_maj * m_intr.transmittance;
                        let flight_pdf = spectra_pdf[0];

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

                    let mis_u = math::guarded_div(1.f, math::avg(mis_d + mis_l));
                    emission += gamma * mis_u * l_intr.L;
                } while (false);

                if (scattered || crossed) acc_opt = ctx.accel(trace_ctx.r, trace_ctx.n);
                if (!acc_opt.hasValue || !acc_opt.value.intr_opt.hasValue) {
                    let e_opt = ctx.emitter.sample_infinite(trace_ctx, ctx.sampler.generate_1d());
                    if (e_opt == none) break;
                    let e_intr = e_opt.value;

                    let light = e_intr.light;
                    let lt = *e_intr.local_to_render.get();

                    let l_ctx = lt / trace_ctx;
                    let l_opt = light(l_ctx.r, l_ctx.lambda);
                    if (l_opt == none) continue;
                    let l_intr = l_opt.value;

                    mis_e *= specular ? 0.f : math::guarded_div(e_intr.pdf * l_intr.pdf, p);
                    let mis_w = math::guarded_div(1.f, math::avg(mis_s + mis_e));
                    emission += beta * mis_w * l_intr.L;
                    break;
                }

                let acc = acc_opt.value;
                var intr = acc.intr_opt.value;
                let div = acc.divider;
                let lt = *div.local_to_render.get();

                if (scattered || crossed) {
                    intr.p = (lt * fv4(intr.p, 1.f)).xyz;
                    intr.n = normalize(lt * intr.n);
                    trace_ctx.inside = dot(-trace_ctx.r.d, intr.n) < 0.f;
                    medium = trace_ctx.inside ? div.int_medium : div.ext_medium;
                    medium_to_render = trace_ctx.inside ? div.int_to_render : div.ext_to_render;

                    let flip_n = trace_ctx.inside ? -1.f : 1.f;
                    intr.n *= flip_n;
                    intr.tn = normalize((lt * fv4(intr.tn * flip_n, 0.f)).xyz);
                    intr.bn = normalize((lt * fv4(intr.bn * flip_n, 0.f)).xyz);
                }

                let mt = *medium_to_render.get();
                let m_ctx = mt / trace_ctx;
                let m_opt = medium.sample(m_ctx, intr.t, ctx.sampler.generate_1d());
                if (m_opt == none) break;
                var m_intr = m_opt.value;
                m_intr.p = (mt * fv4(m_intr.p, 1.f)).xyz;

                let hit = m_intr.t >= intr.t;
                let spectra_pdf = hit
                ? m_intr.transmittance
                : m_intr.sigma_maj * m_intr.transmittance;
                let flight_pdf = spectra_pdf[0];

                beta *= m_intr.transmittance / flight_pdf;
                mis_s *= spectra_pdf / flight_pdf;
                mis_e *= spectra_pdf / flight_pdf;

                if (!hit) {
                    let mis_a = math::guarded_div(1.f, math::avg(mis_s));
                    emission += mis_a * beta * m_intr.sigma_a * m_intr.sigma_e;

                    let p_a = math::guarded_div(m_intr.sigma_a[0], m_intr.sigma_maj[0]);
                    let p_s = math::guarded_div(m_intr.sigma_s[0], m_intr.sigma_maj[0]);
                    let p_n = math::guarded_div(m_intr.sigma_n[0], m_intr.sigma_maj[0]);

                    let u = ctx.sampler.generate_1d();
                    let mode = math::Discrete_Distribution<3>({p_a, p_s, p_n}).sample(u);
                    if (mode == 0) {
                        beta *= m_intr.sigma_a / p_a;
                        break;
                    } else if (mode == 1) {
                        phase = m_intr.phase;
                        let pt = math::Transform(
                            fq::from_rotation_between(-trace_ctx.r.d, {0.f, 1.f, 0.f}).transform()
                        );

                        let p_ctx = pt * trace_ctx;
                        let p_opt = phase.sample(p_ctx, ctx.sampler.generate_2d());
                        if (p_opt == none) break;
                        var p_intr = p_opt.value;
                        p_intr.wi = normalize((pt / fv4(p_intr.wi, 0.f)).xyz);

                        beta *= m_intr.sigma_s / p_s * p_intr.f / p_intr.pdf;
                        mis_s *= m_intr.sigma_s / m_intr.sigma_maj / p_s;
                        mis_e = mis_s;

                        scattered = true;
                        specular = false;
                        crossed = false;
                        f = p_intr.f;
                        p = p_intr.pdf;
                        history_ctx = trace_ctx;
                        trace_ctx = {{m_intr.p, p_intr.wi}, {}, ctx.lambda};
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
                    let st = math::Transform(
                        fq::from_rotation_between(ddiff.r.d, normalize(intr.p)).transform()
                    );
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
                    mis_e *= specular ? 0.f : math::guarded_div(intr.pdf * 1.f, p);
                    let mis_w = math::guarded_div(1.f, math::avg(mis_s + mis_e));
                    emission += mis_w * beta * mat_intr.emission;
                }

                let tbn = transpose(fm33(intr.tn, intr.bn, intr.n));
                intr.n = mul(tbn, mat_intr.normal);

                if (mat_intr.degraded && !math::constant(ctx.lambda)) {
                    emission = fv4(emission[0]); beta = fv4(beta[0]);
                    mis_s = fv4(mis_s[0]); mis_e = fv4(mis_e[0]);
                    trace_ctx.lambda = fv4(ctx.lambda[0]);
                    history_ctx.lambda = trace_ctx.lambda;
                }

                bsdf = mat_intr.bsdf;
                let bt = math::Transform(
                    fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f}).transform()
                );
                let uc = ctx.sampler.generate_1d();
                let u = ctx.sampler.generate_2d();

                var b_ctx = bt * trace_ctx;
                b_ctx.r.d = normalize(b_ctx.r.d);
                let b_opt = bsdf.sample(b_ctx, {uc, u[0], u[1]});
                if (b_opt == none) break;
                var b_intr = b_opt.value;

                if (all(b_ctx.r.d == b_intr.wi)) {
                    scattered = false;
                    crossed = true;
                    history_ctx = trace_ctx;
                    trace_ctx.r.o = intr.p - 0.001f * intr.n;
                    continue;
                }

                scattered = true;
                specular = (bsdf.flags() & mtt::bsdf::Flags::specular) != 0;
                crossed = b_intr.wi[1] < 0.f;

                let trace_n = (crossed ? -1.f : 1.f) * intr.n;
                let trace_p = intr.p + 0.001f * trace_n;
                b_intr.wi = normalize((bt / fv4(b_intr.wi, 0.f)).xyz);
                b_intr.f *= abs(dot(b_intr.wi, trace_n));

                history_ctx.r = trace_ctx.r;
                history_ctx.n = trace_ctx.n;
                trace_ctx.r = {trace_p, b_intr.wi};
                trace_ctx.n = trace_n;
                beta *= b_intr.f / b_intr.pdf;
                mis_e = mis_s;
                f = b_intr.f;
                p = b_intr.pdf;
            }

            if (any(isnan(emission) || isinf(emission))) return {};
            return spectra::Stochastic_Spectrum(trace_ctx.lambda, emission);
        }
    }
}
