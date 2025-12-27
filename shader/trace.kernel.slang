import metatron;

namespace mtt {
    struct Integrate {
        photo::Film film;
        monte_carlo::Integrator_Union integrator;
        accel::Acceleration_Union accel;
        emitter::Emitter_Union emitter;
        sampler::Sampler_Union sampler;
        filter::Filter_Union filter;
        photo::Lens_Union lens;
        u32 seed;
        uv2 range;
        math::Transform ct;
        buf<f32> fresnel;
        RWTexture2D image;
    }

    [shader("compute"), numthreads(8, 8, 1)]
    func integrate(
        iv3 global_idx: SV_DispatchThreadID,
        ParameterBlock<Integrate> in,
    ) -> void {
        var value = fv4(0);
        var sp = in.sampler;
        let f = in.filter;
        let pixel = global_idx.xy;
        var size: uv2;
        in.image.GetDimensions(size.x, size.y);
        bsdf::fresnel_reflectance_table = in.fresnel;

        for (var i = in.range.x; i < in.range.y; ++i) {
            sp.start({pixel, size, i, in.film.spp, 0, in.seed});
            let fixel = in.film(f, pixel, size, sp.generate_2d());
            let camera: photo::Camera;
            let s_opt = camera.sample(in.lens, fixel.position, fixel.dxdy, sp.generate_2d());
            if (s_opt == none) continue;

            var s = s_opt.value;
            s.ray_differential = in.ct / s.ray_differential;
            s.default_differential = in.ct / s.default_differential;
            var st = spectra::Stochastic_Spectrum(sp.generate_1d());
            let a_opt = in.accel.trace(s.ray_differential.r, 0);

            if (a_opt == none || a_opt.value.intr_opt == none) {
                let e_opt = in.emitter.sample_infinite({}, sp.generate_1d());
                if (e_opt == none) continue;
                let e = e_opt.value;

                let r = *e.local_to_render.get() / s.ray_differential.r;
                let l_opt = e.light(r, st.lambda);
                if (l_opt == none) continue;

                let l = l_opt.value;
                st.value = l.L;
                let rgb = fixel(in.film, st);
                value += fv4(rgb / s.pdf * fixel.weight, fixel.weight);
                continue;
            }

            let a_intr = a_opt.value;
            var intr = a_intr.intr_opt.value;
            let t = *a_intr.divider.local_to_render.get();
            let mat = a_intr.divider.material;
            let ctx = t / math::Context(s.ray_differential.r, 0, st.lambda, false,);

            let mat_opt = mat.sample(ctx, {intr.uv});
            if (mat_opt == none) continue;
            let mat_intr = mat_opt.value;
            let tbn = transpose(fm33(intr.tn, intr.bn, intr.n));
            intr.n = mul(tbn, mat_intr.normal);

            let bt = math::Transform(
                fq::from_rotation_between(intr.n, {0.f, 1.f, 0.f}).transform()
            );
            var b_ctx = bt * ctx;
            let uc = sp.generate_1d();
            let u = sp.generate_2d();
            b_ctx.r.d = normalize(b_ctx.r.d);
            let b_opt = mat_intr.bsdf.sample(b_ctx, {uc, u[0], u[1]});
            if (b_opt == none) continue;
            let b_intr = b_opt.value;

            st.value = b_intr.f / b_intr.pdf;
            let rgb = fixel(in.film, st);
            value += fv4(rgb / s.pdf * fixel.weight, fixel.weight);
        }

        in.image.Store(pixel, value / max(value.w, math::epsilon));
    }
}
