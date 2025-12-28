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
            var terminated = false;

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
            let rdiff = ctx.ray_differential;
            let ddiff = ctx.default_differential;
            let ct = ctx.render_to_camera;

            while (true) {
                depth += u32(scattered);
                if (terminated || depth >= ctx.max_depth) break;

                let q = math::max(beta * math::guarded_div(1.f, math::avg(mis_s)));
                if (q < 1.f && depth > 1uz) {
                    let rr_u = ctx.sampler.generate_1d();
                    if (rr_u > q) {
                        terminated = true; continue;
                    } else beta /= q;
                }


                if (scattered || crossed) acc_opt = ctx.accel(trace_ctx.r, trace_ctx.n);
                if (true) {
                // if (!acc_opt.hasValue || !acc_opt.value.intr_opt.hasValue) {
                    terminated = true;

                    let e_opt = ctx.emitter.sample_infinite(trace_ctx, ctx.sampler.generate_1d());
                    if (e_opt == none) continue;
                    let e_intr = e_opt.value;

                    let light = e_intr.light;
                    let lt = *e_intr.local_to_render.get();

                    let l_ctx = lt / trace_ctx;
                    let l_opt = light(l_ctx.r, l_ctx.lambda);
                    if (l_opt == none) continue;
                    let l_intr = l_opt.value;

                    mis_e *= specular ? 0.f : math::guarded_div(e_intr.pdf * l_intr.pdf, p);
                    let mis_w = 1.f; // math::guarded_div(1.f, math::avg(mis_s + mis_e));
                    emission += beta * mis_w * l_intr.L;
                    continue;
                }
            }

            if (any(isnan(emission)) || any(isinf(emission))) return {};
            return spectra::Stochastic_Spectrum(ctx.lambda, emission);
        }
    }
}
