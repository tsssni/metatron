import metatron;

namespace mtt {
    static u32 half_bits = 16;
    static u32 half_mask = (1u << 16) - 1;

    public struct Restir_Integrator {
        public struct Estimation {
            public spectra::Stochastic_Spectrum Li = {};
            public fv4 beta = {};
            public f32 mis = 0.f;
            public f32 p_e = 0.f;
            public fv3 wi = {};
            public u32 end = 0;
            public f32 W = 0.f;
            public f32 u = 0.f;
        }

        public struct Path {
            public math::Reservoir r = {};
            public fv4 Li = {};
            public fv4 beta = {};
            public fv4 gamma = {};
            public fv3 p = {};
            public u32 pixel = 0;
            public fv3 wi = {};
            public f32 J = 0.f;
            public fv3 we = {};
            public f32 mis = 0.f;
            public fv3 pdf = {};
            public u32 depth = 0;
            public __init() {}

            [mutating] public void merge(Path o, f32 u) {
                if (this.r(o.r, u)) {
                    let res = this.r;
                    this = o;
                    this.r = res;
                }
            }
            [mutating] public void estimate(Estimation e) {
                let y = math::max(e.mis * e.beta * e.Li.value);
                var r: math::Reservoir;
                r.p_hat = y;
                r.M = 0.f;
                r.W = e.W;
                r.shift(1.f, 1.f);
                if (this.r(r, e.u)) {
                    this.Li = e.beta * e.Li.value;
                    this.gamma = e.beta;
                    this.we = e.wi;
                    this.mis = e.mis;
                    this.pdf[2] = e.p_e;
                    this.depth = (this.depth & half_mask) | (e.end << half_bits);
                }
            }
        }

        public struct Shift {
            public Path path = {};
            public f32 J = 1.f;
        }

        public Array<buf<Path>, 2> pathes;
        public u32 reuse_iterations = 5;
        public u32 spatial_samples = 4;
        public u32 spatial_radius = 16;
        public f32 near_field_distance = 1e-1f;

        public uptr integrate = 0;
        public uptr restir = 0;
        public uptr arguments = 0;
        public uptr constants = 0;

        // gris: https://graphics.cs.utah.edu/research/projects/gris/
        public func sample(monte_carlo::Ray r) -> Optional<Path> {
            var path: Path;
            path.pixel = (r.pixel[0] & half_mask) | (r.pixel[1] << 16);
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
            trace_ctx.r = r.ray_differential.r;
            trace_ctx.lambda = r.lambda;

            var acc_opt: Optional<accel::Interaction>;
            var rdiff = r.ray_differential;
            var ddiff = r.default_differential;
            let ct = r.render_to_camera;

            while (true) {
                depth += 1u;
                if (depth >= r.max_depth) break;

                let q = math::max(beta * rr);
                let rru = r.sampler.generate_1d();
                if (q < 1.f) {
                    if (rru > q) break;
                    else rr /= q;
                }

                do {
                    if (!scattered) break;
                    var direct_ctx = trace_ctx;

                    let u = fv4(r.sampler.generate_2d(), r.sampler.generate_2d());
                    if (specular) break;
                    let e_opt = r.emitter.sample(direct_ctx, u[0]);
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
                    let mis_u = 1.f / (mis_d + 1.f);
                    acc_opt = r.accel(direct_ctx.r, direct_ctx.n);
                    if (!acc_opt.hasValue || !acc_opt.value.valid) {
                        path.estimate(Estimation(
                            spectra::Stochastic_Spectrum(trace_ctx.lambda, l_intr.L),
                            gamma, mis_u, p_e, l_intr.wi, depth, rr, u[3]
                        ));
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
                    path.estimate(Estimation(
                        spectra::Stochastic_Spectrum(trace_ctx.lambda, l_intr.L),
                        gamma, mis_u, p_e, l_intr.wi, depth, rr, u[3]
                    ));
                } while (false);

                acc_opt = r.accel(trace_ctx.r, trace_ctx.n);
                if (!acc_opt.hasValue || !acc_opt.value.valid) {
                    let eu = r.sampler.generate_1d();
                    let e_opt = r.emitter.sample_infinite(trace_ctx, r.sampler.generate_1d());
                    if (e_opt == none) break;
                    let e_intr = e_opt.value;

                    let light = e_intr.light;
                    let lt = *e_intr.local_to_render.get();

                    let l_ctx = lt / trace_ctx;
                    let l_opt = light(l_ctx.r, l_ctx.lambda);
                    if (l_opt == none) break;
                    let l_intr = l_opt.value;

                    let p_e = e_intr.pdf * l_intr.pdf;
                    let mis_e = specular ? 0.f : math::guarded_div(p_e, p);
                    let mis_w = 1.f / (1.f + mis_e);
                    path.estimate(Estimation(
                        spectra::Stochastic_Spectrum(trace_ctx.lambda, l_intr.L),
                        beta, mis_w, -p_e, path.wi, depth, rr, eu
                    ));
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
                    let eu = r.sampler.generate_1d();
                    let mis_e = specular ? 0.f : math::guarded_div(intr.pdf * 1.f, p);
                    let mis_w = 1.f / (1.f + mis_e);
                    path.estimate(Estimation(
                        spectra::Stochastic_Spectrum(trace_ctx.lambda, mat_intr.emission),
                        beta, mis_w, -intr.pdf, path.wi, depth, rr, eu
                    ));
                }

                let tbn = transpose(fm33(intr.tn, intr.bn, intr.n));
                intr.n = mul(tbn, mat_intr.normal);

                bsdf = mat_intr.bsdf;
                let btm = fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f}).transform();
                let bt = math::Transform(btm, transpose(btm));
                let cu = r.sampler.generate_1d();
                let du = r.sampler.generate_2d();

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

                if (mat_intr.degraded && !reflected && !math::constant(trace_ctx.lambda)) {
                    beta = fv4(beta[0]); f = fv4(f[0]);
                    trace_ctx.lambda = fv4(trace_ctx.lambda[0]);
                    history_ctx.lambda = trace_ctx.lambda;
                }

                if (connectable && b_intr.connectable && intr.t > near_field_distance) {
                    path.beta = beta;
                    path.p = intr.p;
                    path.wi = b_intr.wi;
                    path.J = math::sqr(intr.t) / abs(math::unit_to_cos_theta(b_ctx.r.d));
                    path.pdf[0] = p;
                    path.pdf[1] = b_intr.pdf;
                    path.depth = (path.depth & ~half_mask) | (depth - 1);
                }

                f = b_intr.f;
                p = b_intr.pdf;
                connectable = ((path.depth & half_mask) == 0) && b_intr.connectable;
            }

            if (!all(isfinite(path.Li))) return {};
            path.r.M = 1.f;
            path.r.finalize();
            return path;
        }

        public func replay(monte_carlo::Ray r, Path np, f32 u) -> Optional<Shift> {
            var p = np;
            var J = 1.f;
            let connection = p.depth & half_mask;
            let end = p.depth >> half_bits;
            var depth = 0u;
            var beta = fv4(1.f);
            var connected = false;

            var trace_ctx: math::Context;
            var history_ctx: math::Context;
            trace_ctx.r = r.ray_differential.r;
            trace_ctx.lambda = r.lambda;

            var rdiff = r.ray_differential;
            var ddiff = r.default_differential;

            while (true) {
                depth += 1;
                if (depth > connection + u32(connected)) break;
                if (depth > 1) { r.sampler.generate_2d(); r.sampler.generate_2d(); }
                r.sampler.generate_1d();

                let acc_opt = r.accel(trace_ctx.r, trace_ctx.n);
                if (!acc_opt.hasValue || !acc_opt.value.valid) break;
                let acc = acc_opt.value;
                var intr = acc.intr;
                if (connected) {
                    let rt = length(p.p - trace_ctx.r.o);
                    if (rt - intr.t > 0.001f * rt) break;
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
                let cu = r.sampler.generate_1d();
                let du = r.sampler.generate_2d();

                var b_intr: mtt::bsdf::Interaction;
                let delta = abs(p.mis) == 1.f;
                let mised = connection + 2 == end;
                let direct = mised && p.pdf[2] > 0.f;

                if (depth == connection) {
                    let rt = length(p.p - intr.p);
                    if (rt < near_field_distance) break;
                    let wi = (bt * fv4((p.p - intr.p) / rt, 0.f)).xyz;
                    let c_opt = bsdf(b_ctx.r.d, wi, cu);
                    if (c_opt == none) break;
                    b_intr = c_opt.value;
                    J *= b_intr.pdf / p.pdf[0] * p.J;
                    p.pdf[0] = b_intr.pdf;
                    connected = end > connection + 1;
                } else if (depth == connection + 1) {
                    let wi = (bt * fv4(mised ? p.we : p.wi, 0.f)).xyz;
                    let c_opt = bsdf(b_ctx.r.d, wi, direct ? -1.f : cu);
                    if (c_opt == none) break;
                    b_intr = c_opt.value;
                    b_intr.f *= abs(math::unit_to_cos_theta(b_intr.wi));

                    let pdf = direct ? p.pdf[2] : b_intr.pdf;
                    let jdiv = direct ? 1.f : b_intr.pdf / p.pdf[1];
                    let geo = math::sqr(intr.t) / abs(math::unit_to_cos_theta(b_ctx.r.d));
                    p.pdf[1] = b_intr.pdf;

                    let mis_r = delta ? 0.f : !direct
                    ? math::guarded_div(-p.pdf[2], b_intr.pdf)
                    : math::guarded_div(b_intr.pdf, p.pdf[2]);
                    p.mis = mised ? 1.f / (1.f + mis_r) : p.mis;

                    beta *= b_intr.f / pdf;
                    let base = mised ? p.gamma : p.beta;
                    p.Li *= beta / base;
                    if (mised) p.gamma = beta;
                    else p.beta = beta;
                    J *= jdiv / geo;
                    p.J = geo;
                    p.r.p_hat = math::max(p.mis * p.Li);
                    return Shift(p, J);
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

        var r: monte_carlo::Ray;
        r.accel = cb.accel;
        r.emitter = cb.emitter;
        r.sampler = sp;
        r.lambda = spec.lambda;
        r.ray_differential = s.ray_differential;
        r.default_differential = s.default_differential;
        r.render_to_camera = cb.ct;
        r.pixel = pixel;
        r.size = size;
        r.sample_index = cb.sample_index;
        r.max_depth = film.depth;

        let intg = tag<Restir_Integrator>(cb.integrator).get();
        var p: Restir_Integrator.Path;
        p.pixel = (u32(pixel.x) & 0xffffu) | (u32(pixel.y) << 16);
        let p_opt = intg.sample(r);
        if (p_opt != none) p = p_opt.value;
        if (intg.reuse_iterations == 0) {
            p.Li *= p.mis * p.r.W / s.pdf;
            let rgb = fixel(film, spectra::Stochastic_Spectrum(spec.lambda, p.Li));
            let value = fv4(rgb * fixel.weight, fixel.weight);
            let sampled = cb.image.Load(pixel) + value;
            cb.image.Store(pixel, sampled);
        } else intg.pathes[0][u32(pixel.y) * size.x + u32(pixel.x)] = p;
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

        var spec = fv4(0.f);
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
            sp.ctx.dim = 4;
            sp.start();
            spec = spectra::Stochastic_Spectrum(sp.generate_1d()).lambda;
            var r: monte_carlo::Ray;
            r.accel = cb.accel;
            r.emitter = cb.emitter;
            r.sampler = sp;
            r.lambda = spec;
            r.ray_differential = s.ray_differential;
            r.default_differential = s.default_differential;
            r.render_to_camera = cb.ct;
            r.pixel = pixel;
            r.size = size;
            r.sample_index = cb.sample_index;
            r.max_depth = film.depth;
            let c_opt = intg.replay(r, np, u[1]);
            if (c_opt == none) continue;
            var c = c_opt.value.path;
            let cJ = c_opt.value.J;

            sp.ctx.pixel = pw;
            sp.ctx.dim = 4;
            sp.start();
            spec = spectra::Stochastic_Spectrum(sp.generate_1d()).lambda;
            var nr: monte_carlo::Ray;
            nr.accel = cb.accel;
            nr.emitter = cb.emitter;
            nr.sampler = sp;
            nr.lambda = spec;
            nr.ray_differential = ns.ray_differential;
            nr.default_differential = ns.default_differential;
            nr.render_to_camera = cb.ct;
            nr.pixel = py;
            nr.size = size;
            nr.sample_index = cb.sample_index;
            nr.max_depth = film.depth;
            let d_opt = intg.replay(nr, p, u[2]);
            if (d_opt == none) continue;
            var d = d_opt.value.path;
            let dJ = d_opt.value.J;

            if (false
            || !isfinite(cJ) || !isfinite(dJ)
            || !isfinite(d.r.p_hat)
            || !all(isfinite(c.Li))) continue;

            valid += 1;
            let Mi = np.r.M;
            let Mc = p.r.M;
            let pni_hat = math::guarded_div(np.r.p_hat, cJ);
            let pnc_hat = c.r.p_hat;
            let mi = math::guarded_div(Mi * pni_hat, Mi * pni_hat + Mc / N * pnc_hat);
            let pci_hat = d.r.p_hat * dJ;
            let pcc_hat = p.r.p_hat;
            canonical += math::guarded_div(Mc / N * pcc_hat, Mi * pci_hat + Mc / N * pcc_hat);

            c.r.shift(cJ, mi);
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
            let dz = uv2(u32(dst.pixel) & half_mask, u32(dst.pixel) >> half_bits);
            sp.ctx.pixel = dz;
            sp.ctx.dim = 4;
            sp.start();
            var Li = spectra::Stochastic_Spectrum(sp.generate_1d());
            Li.value = dst.Li * dst.mis * dst.r.W / s.pdf;
            let rgb = fixel(film, Li);
            let value = fv4(rgb * fixel.weight, fixel.weight);
            let sampled = cb.image.Load(pixel) + value;
            cb.image.Store(pixel, sampled);
        }
    }
}
