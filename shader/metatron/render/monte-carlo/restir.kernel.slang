import metatron;

namespace mtt {
    public struct Restir_Integrator {
        public struct Path {
            public math::Reservoir r = {};
            public spectra::Stochastic_Spectrum Li = {};
            public fv4 beta = {};
            public fv3 p = {};
            public f32 t = 0.f;
            public fv3 wi = {};
            public f32 cos_theta = 0.f;
            public fv2 pdf = {};
            public f32 J = 1.f;
            public u32 pixel = 0;
            public u32 depth = 0;
            public uv3 padding = 0;
            public __init() {}

            [mutating] public void merge(Path o, f32 u) {
                if (this.r(o.r, u)) {
                    let res = this.r;
                    this = o;
                    this.r = res;
                }
            }
            [mutating] public void operator()(spectra::Stochastic_Spectrum Li, f32 W, f32 u) {
                let y = math::max(Li.value);
                var r: math::Reservoir;
                r.p_hat = y;
                r.M = 1.f;
                r.W = W;
                r.shift(1.f, 1.f);
                if (this.r(r, u)) this.Li = Li;
            }
        }

        public uptr tracer = 0;
        public uptr reuser = 0;
        public uptr constants = 0;
        public buf<Path> pathes[2];
        public u32 reuse_iterations = 5;
        public u32 spatial_samples = 4;
        public u32 spatial_radius = 16;

        // gris: https://graphics.cs.utah.edu/research/projects/gris/
        public func sample(monte_carlo::Context ctx) -> Optional<Path> {
            static f32 reconnection_t = 1e-2f;
            static i32 half_mask = (1 << 16) - 1;
            var path: Path;
            path.Li = spectra::Stochastic_Spectrum(ctx.lambda, fv4(0.f));
            path.pixel = (ctx.pixel[0] & half_mask) | (ctx.pixel[1] << 16);
            var beta = fv4(1.f);

            var depth = 0u;
            var scattered = false;
            var specular = false;
            var connectable = false;

            var p = 0.f;
            var rr = 1.f;
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
                depth += 1u;
                if (depth >= ctx.max_depth) break;

                let q = math::max(beta * rr);
                if (q < 1.f) {
                    let rr_u = ctx.sampler.generate_1d();
                    if (rr_u > q) break;
                    else rr /= q;
                }

                do {
                    if (!scattered) break;
                    var direct_ctx = trace_ctx;

                    let u = fv4(ctx.sampler.generate_2d(), ctx.sampler.generate_2d());
                    if (specular) break;
                    let e_opt = ctx.emitter.sample(direct_ctx, u[0]);
                    if (e_opt == none) break;
                    let e_intr = e_opt.value;
                    let et = *e_intr.local_to_render.get();

                    let light = e_intr.light;
                    let l_ctx = et / direct_ctx;
                    let l_opt = light.sample(l_ctx, {u[1], u[2]});
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
                        path(spectra::Stochastic_Spectrum(trace_ctx.lambda, mis_u * gamma * l_intr.L), rr, u[3]);
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

                    let light_before_surface = l_intr.t < intr.t - 0.001f;
                    let close_to_light = length(intr.p - l_intr.p) < 0.001f;
                    let is_emissive = bool(div.material.flags() & material::Flags::emissive);
                    if (!light_before_surface && (!is_emissive || !close_to_light)) break;
                    else if (!light_before_surface) {
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
                    path(spectra::Stochastic_Spectrum(trace_ctx.lambda, mis_u * gamma * l_intr.L), rr, u[3]);
                } while (false);

                acc_opt = ctx.accel(trace_ctx.r, trace_ctx.n);
                let eu = ctx.sampler.generate_1d();
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
                    path(spectra::Stochastic_Spectrum(trace_ctx.lambda, beta * mis_w * l_intr.L), rr, eu);
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
                    path(spectra::Stochastic_Spectrum(trace_ctx.lambda, beta * mis_w * mat_intr.emission), rr, eu);
                }

                let tbn = transpose(fm33(intr.tn, intr.bn, intr.n));
                intr.n = mul(tbn, mat_intr.normal);

                bsdf = mat_intr.bsdf;
                let btm = fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f}).transform();
                let bt = math::Transform(btm, transpose(btm));
                let cu = ctx.sampler.generate_1d();
                let du = ctx.sampler.generate_2d();

                var b_ctx = bt * trace_ctx;
                b_ctx.r.d = normalize(b_ctx.r.d);
                let b_opt = bsdf.sample(b_ctx, {cu, du[0], du[1]});
                if (b_opt == none) break;
                var b_intr = b_opt.value;

                scattered = true;
                specular = (bsdf.flags() & mtt::bsdf::Flags::specular) != 0;
                let reflected = b_intr.wi[1] >= 0.f;
                let trace_n = (b_intr.wi[1] < 0.f ? -1.f : 1.f) * intr.n;
                let trace_p = intr.p + 0.001f * trace_n;
                b_intr.f *= abs(math::unit_to_cos_theta(b_intr.wi));
                b_intr.wi = normalize((bt / fv4(b_intr.wi, 0.f)).xyz);

                beta *= b_intr.f / b_intr.pdf;
                history_ctx.r = trace_ctx.r;
                history_ctx.n = trace_ctx.n;
                trace_ctx.r = {trace_p, b_intr.wi};
                trace_ctx.n = trace_n;
                f = b_intr.f;
                p = b_intr.pdf;

                if (mat_intr.degraded && !reflected && !math::constant(trace_ctx.lambda)) {
                    beta = fv4(beta[0]); f = fv4(f[0]);
                    trace_ctx.lambda = fv4(trace_ctx.lambda[0]);
                    history_ctx.lambda = trace_ctx.lambda;
                }

                if (connectable && b_intr.connectable && intr.t > reconnection_t) {
                    path.beta = beta;
                    path.p = intr.p;
                    path.t = intr.t;
                    path.wi = b_intr.wi;
                    path.cos_theta = abs(math::unit_to_cos_theta(b_ctx.r.d));
                    path.pdf = {p, b_intr.pdf};
                    path.depth = depth - 1;
                }
                connectable = (path.depth == 0) && b_intr.connectable;
            }

            // support set of different path depth is not intersected
            if (!all(isfinite(path.Li.value))) return {};
            path.r.M = 1.f;
            path.r.finalize();
            return path;
        }

        public func replay(monte_carlo::Context ctx, Path np, f32 u) -> Optional<Path> {
            var p = np;
            p.J = 1.f;
            var depth = 0u;
            var beta = fv4(1.f);
            var connected = false;

            var trace_ctx: math::Context;
            var history_ctx: math::Context;
            trace_ctx.r = ctx.ray_differential.r;
            trace_ctx.lambda = ctx.lambda;

            var rdiff = ctx.ray_differential;
            var ddiff = ctx.default_differential;

            while (true) {
                depth += 1;
                if (depth > p.depth + u32(connected)) break;
                // skip direct lighting
                if (depth > 1) { ctx.sampler.generate_2d(); ctx.sampler.generate_2d(); }
                // skip final lighting
                ctx.sampler.generate_1d();

                let acc_opt = ctx.accel(trace_ctx.r, trace_ctx.n);
                if (!acc_opt.hasValue || !acc_opt.value.valid) break;
                let acc = acc_opt.value;
                var intr = acc.intr;
                if (connected) {
                    let rt = length(p.p - trace_ctx.r.o);
                    if (rt - intr.t > 1e-3f) break;
                }
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
                let tbn = transpose(fm33(intr.tn, intr.bn, intr.n));
                intr.n = mul(tbn, mat_intr.normal);

                if (mat_intr.degraded && !math::constant(trace_ctx.lambda)) {
                    beta = fv4(beta[0]);
                    trace_ctx.lambda = fv4(trace_ctx.lambda[0]);
                    history_ctx.lambda = trace_ctx.lambda;
                }

                var bsdf = mat_intr.bsdf;
                let btm = fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f}).transform();
                let bt = math::Transform(btm, transpose(btm));
                var b_ctx = bt * trace_ctx;
                b_ctx.r.d = normalize(b_ctx.r.d);
                let cu = ctx.sampler.generate_1d();
                let du = ctx.sampler.generate_2d();

                var b_intr: mtt::bsdf::Interaction;
                if (depth == p.depth) {
                    var wi = normalize(p.p - intr.p);
                    wi = (bt * fv4(wi, 0.f)).xyz;
                    let c_opt = bsdf(b_ctx.r.d, wi, cu);
                    if (c_opt == none) break;
                    b_intr = c_opt.value;
                    p.J *= b_intr.pdf / math::prod(p.pdf) * math::sqr(p.t) / p.cos_theta;
                    p.pdf[0] = b_intr.pdf;
                    connected = true;
                } else if (depth == p.depth + 1) {
                    let wi = (bt * fv4(p.wi, 0.f)).xyz;
                    let c_opt = bsdf(b_ctx.r.d, wi, cu);
                    if (c_opt == none) break;
                    b_intr = c_opt.value;
                    b_intr.f *= abs(math::unit_to_cos_theta(b_intr.wi));
                    beta *= b_intr.f / b_intr.pdf;
                    p.Li.value = p.Li.value / p.beta * beta;
                    p.beta = beta;
                    p.t = intr.t;
                    p.pdf[1] = b_intr.pdf;
                    p.cos_theta = abs(math::unit_to_cos_theta(b_ctx.r.d));
                    p.J *= b_intr.pdf / math::sqr(p.t) * p.cos_theta;
                    p.r.p_hat = math::max(p.Li.value);
                    return p;
                } else {
                    let c_opt = bsdf.sample(b_ctx, {cu, du[0], du[1]});
                    if (c_opt == none) break;
                    b_intr = c_opt.value;
                }

                let trace_n = (b_intr.wi[1] < 0.f ? -1.f : 1.f) * intr.n;
                let trace_p = intr.p + 0.001f * trace_n;
                b_intr.f *= abs(math::unit_to_cos_theta(b_intr.wi));
                b_intr.wi = normalize((bt / fv4(b_intr.wi, 0.f)).xyz);

                beta *= b_intr.f / b_intr.pdf;
                history_ctx.r = trace_ctx.r;
                history_ctx.n = trace_ctx.n;
                trace_ctx.r = {trace_p, b_intr.wi};
                trace_ctx.n = trace_n;
            }

            return {};
        }
    }

    struct Constants {
        accel::Acceleration accel;
        emitter::Emitter emitter;
        sampler::Sampler sampler;
        filter::Filter filter;
        photo::Lens lens;
        photo::proxy::Film film;
        math::Transform ct;
        u32 seed;
        u32 sample_index;
        u32 iter;
        u32 integrator;
        [vk_binding(1, 0)] RWTexture2D image;
    }
    [vk_binding(0, 0)] ParameterBlock<Constants> constants;

    [shader("compute"), numthreads(8, 8, 1)]
    func trace(
        iv3 global_idx: SV_DispatchThreadID,
    ) -> void {
        let pixel = global_idx.xy;
        let cb = constants;
        let film = cb.film.get();
        var size: uv2;
        cb.image.GetDimensions(size.x, size.y);
        if (any(pixel != clamp(pixel, 0, size - 1))) return;

        var sp: sampler::proxy::Sampler;
        sp.ctx.pixel = pixel;
        sp.ctx.size = size;
        sp.ctx.idx = cb.sample_index;
        sp.ctx.spp = film.spp;
        sp.ctx.dim = 0;
        sp.ctx.seed = cb.seed;
        sp.sampler = cb.sampler;
        sp.start();

        let fixel = film(cb.filter, pixel, size, sp.generate_2d());
        let camera: photo::Camera;
        let s_opt = camera.sample(cb.lens, fixel.position, fixel.dxdy, sp.generate_2d());
        if (s_opt == none) return;
        var s = s_opt.value;
        s.ray_differential = cb.ct / s.ray_differential;
        s.default_differential = cb.ct / s.default_differential;
        let spec = spectra::Stochastic_Spectrum(sp.generate_1d());

        var ctx: monte_carlo::Context;
        ctx.accel = cb.accel;
        ctx.emitter = cb.emitter;
        ctx.sampler = sp;
        ctx.lambda = spec.lambda;
        ctx.ray_differential = s.ray_differential;
        ctx.default_differential = s.default_differential;
        ctx.render_to_camera = cb.ct;
        ctx.pixel = pixel;
        ctx.sample_index = cb.sample_index;
        ctx.max_depth = film.depth;

        let intg = tag<Restir_Integrator>(cb.integrator).get();
        var path: Restir_Integrator.Path;
        path.Li = spectra::Stochastic_Spectrum(spec.lambda, fv4(0.f));
        path.pixel = (u32(pixel.x) & 0xffffu) | (u32(pixel.y) << 16);
        let p_opt = intg.sample(ctx);
        if (p_opt != none) path = p_opt.value;
        intg.pathes[0][u32(pixel.y) * size.x + u32(pixel.x)] = path;
    }

    [shader("compute"), numthreads(8, 8, 1)]
    func reuse(
        iv3 global_idx: SV_DispatchThreadID,
    ) -> void {
        let pixel = global_idx.xy;
        let cb = constants;
        let film = cb.film.get();
        var size: uv2;
        cb.image.GetDimensions(size.x, size.y);
        if (any(pixel != clamp(pixel, 0, size - 1))) return;

        let intg = tag<Restir_Integrator>(cb.integrator).get();
        let iter = cb.iter;
        let N = f32(intg.spatial_samples);
        let camera: photo::Camera;

        var sp: sampler::proxy::Sampler;
        sp.ctx.size = size;
        sp.ctx.idx = cb.sample_index;
        sp.ctx.spp = film.spp;
        sp.ctx.seed = cb.seed;
        sp.sampler = cb.sampler;

        sp.ctx.pixel = pixel;
        sp.ctx.dim = 0;
        sp.start();
        let fixel = film(cb.filter, pixel, size, sp.generate_2d());
        let s_opt = camera.sample(cb.lens, fixel.position, fixel.dxdy, sp.generate_2d());
        if (s_opt == none) return;
        var s = s_opt.value;
        s.ray_differential = cb.ct / s.ray_differential;
        s.default_differential = cb.ct / s.default_differential;

        let pi = u32(pixel.y) * size.x + u32(pixel.x);
        var p = intg.pathes[iter & 1][pi];
        var dst = p;
        dst.r = {};
        var canonical = 1.f;
        var valid = 0u;

        for (u32 i = 0; i < intg.spatial_samples; i += 1) {
            sp.ctx.pixel = pixel;
            sp.ctx.dim = (intg.spatial_samples * 4 + 1) * iter + i * 4;
            sp.start();
            let u = fv4(sp.generate_2d(), sp.generate_2d());
            let theta = u[0] * math::pi * 2.f;
            let py = pixel + iv2(fv2(cos(theta), sin(theta)) * f32(intg.spatial_radius));
            if (any(py != clamp(py, iv2(0), iv2(size) - 1))) continue;

            let ni = u32(py.y) * size.x + u32(py.x);
            let np = intg.pathes[iter & 1][ni];
            let pz = uv2(u32(np.pixel) & 0xffffu, u32(np.pixel) >> 16);
            let pw = uv2(u32(p.pixel) & 0xffffu, u32(p.pixel) >> 16);

            sp.ctx.pixel = py;
            sp.start();
            let gixel = film(cb.filter, py, size, sp.generate_2d());
            let ns_opt = camera.sample(cb.lens, gixel.position, gixel.dxdy, sp.generate_2d());
            if (ns_opt == none) continue;
            var ns = ns_opt.value;
            ns.ray_differential = cb.ct / ns.ray_differential;
            ns.default_differential = cb.ct / ns.default_differential;

            sp.ctx.pixel = pz;
            sp.ctx.dim = 5;
            sp.start();
            var rctx: monte_carlo::Context;
            rctx.accel = cb.accel;
            rctx.emitter = cb.emitter;
            rctx.sampler = sp;
            rctx.lambda = np.Li.lambda;
            rctx.ray_differential = s.ray_differential;
            rctx.default_differential = s.default_differential;
            rctx.render_to_camera = cb.ct;
            rctx.pixel = pixel;
            rctx.sample_index = cb.sample_index;
            rctx.max_depth = film.depth;
            let c_opt = intg.replay(rctx, np, u[1]);
            if (c_opt == none) continue;
            var c = c_opt.value;

            sp.ctx.pixel = pw;
            sp.ctx.dim = 5;
            sp.start();
            var nrctx: monte_carlo::Context;
            nrctx.accel = cb.accel;
            nrctx.emitter = cb.emitter;
            nrctx.sampler = sp;
            nrctx.lambda = p.Li.lambda;
            nrctx.ray_differential = ns.ray_differential;
            nrctx.default_differential = ns.default_differential;
            nrctx.render_to_camera = cb.ct;
            nrctx.pixel = py;
            nrctx.sample_index = cb.sample_index;
            nrctx.max_depth = film.depth;
            let d_opt = intg.replay(nrctx, p, u[2]);
            if (d_opt == none) continue;
            var d = d_opt.value;

            if (false
            || !isfinite(c.J) || !isfinite(d.J)
            || !isfinite(d.r.p_hat)
            || !all(isfinite(c.Li.value))) continue;

            valid += 1;
            let Mi = np.r.M;
            let Mc = p.r.M;
            let pni_hat = math::guarded_div(np.r.p_hat, c.J);
            let pnc_hat = c.r.p_hat;
            let mi = math::guarded_div(Mi * pni_hat, Mi * pni_hat + Mc / N * pnc_hat);
            let pci_hat = d.r.p_hat * d.J;
            let pcc_hat = p.r.p_hat;
            canonical += math::guarded_div(Mc / N * pcc_hat, Mi * pci_hat + Mc / N * pcc_hat);

            c.r.shift(c.J, mi);
            dst.merge(c, u[3]);
        }

        sp.ctx.pixel = pixel;
        sp.ctx.dim = (intg.spatial_samples * 4 + 1) * iter + intg.spatial_samples * 4;
        sp.start();
        p.r.shift(1.f, canonical);
        dst.merge(p, sp.generate_1d());
        dst.r.finalize();
        dst.r.W = math::guarded_div(dst.r.W, f32(valid + 1));

        if (iter < intg.reuse_iterations - 1) {
            intg.pathes[(iter + 1) & 1][pi] = dst;
        } else {
            var Li = dst.Li;
            Li.value *= dst.r.W / s.pdf;
            let rgb = fixel(film, Li);
            let value = fv4(rgb * fixel.weight, fixel.weight);
            let sampled = cb.image.Load(pixel) + value;
            cb.image.Store(pixel, sampled);
        }
    }
}
