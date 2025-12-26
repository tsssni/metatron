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
        RWTexture2D image;
    }

    [shader("compute"), numthreads(8, 8, 1)]
    func integrate(
        iv3 global_idx: SV_DispatchThreadID,
        ParameterBlock<Integrate> in,
    ) -> void {
        var value = fv4(0);
        var sp = in.sampler.get();
        let f = in.filter.get();
        let pixel = global_idx.xy;
        var size: uv2;
        in.image.GetDimensions(size.x, size.y);

        for (var i = in.range.x; i < in.range.y; ++i) {
            sp.start({pixel, size, i, in.film.spp, 0, in.seed});
            let fixel = in.film(f, pixel, size, sp.generate_2d());
            let camera: photo::Camera;
            let s_opt = camera.sample(in.lens.get(), fixel.position, fixel.dxdy, sp.generate_2d());
            if (s_opt == none) continue;

            var s = s_opt.value;
            s.ray_differential = in.ct / s.ray_differential;
            s.default_differential = in.ct / s.default_differential;
            let a_opt = in.accel.get().trace(s.ray_differential.r, 0);

            if (a_opt == none || a_opt.value.intr_opt == none) {
                let e_opt = in.emitter.get().sample_infinite({}, sp.generate_1d());
                if (e_opt == none) continue;
                let e = e_opt.value;

                let r = e.local_to_render.get() / s.ray_differential.r;
                var st = spectra::Stochastic_Spectrum(sp.generate_1d());
                let l_opt = e.light.get()(r, st.lambda);
                if (l_opt == none) continue;

                let l = l_opt.value;
                st.value = l.L;
                let rgb = fixel(in.film, st);
                value += fv4(rgb / s.pdf * fixel.weight, fixel.weight);
                continue;
            }

            let a_intr = a_opt.value;
            let s_intr = a_intr.intr_opt.value;
            value += fv4(s_intr.p * fixel.weight, fixel.weight);
        }

        in.image.Store(pixel, value / value.w);
    }
}
