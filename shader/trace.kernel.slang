import metatron;

namespace mtt {
    struct Integrate {
        math::Transform transform;
        photo::Film film;
        accel::Acceleration_Union accel;
        emitter::Emitter_Union emitter;
        sampler::Sampler_Union sampler;
        photo::Lens_Union lens;
        RWTexture2D image;
    }

    [shader("compute"), numthreads(8, 8, 1)]
    func integrate(
        iv3 global_idx: SV_DispatchThreadID,
        ParameterBlock<Integrate> in,
    ) -> void {
        let pixel = global_idx.xy;
        var size: uv2;
        in.image.GetDimensions(size.x, size.y);

        for (var i = 0; i < in.film.spp; ++i) {
            var sp = in.sampler.get();
            sp.start({pixel, size, i, in.film.spp, 0, 0});
            let fixel = in.film(0, pixel, size, sp.generate_2d());
            let camera: photo::Camera;
            let s_opt = camera.sample(in.lens.get(), fixel.position, fixel.dxdy, sp.generate_2d());
            if (s_opt == none) {
                in.image.Store(pixel, 0);
                return;
            }

            var s = s_opt.value;
            s.ray_differential = in.transform / s.ray_differential;
            s.default_differential = in.transform / s.default_differential;
            let a_opt = in.accel.get().trace(s.ray_differential.r, 0);

            if (a_opt == none || a_opt.value.intr_opt == none) {
                let e = in.emitter.get().sample_infinite({}, 0).value;
                let r = e.local_to_render.get() / s.ray_differential.r;
                var st = spectra::Stochastic_Spectrum(sp.generate_1d(), 0);
                st.value = e.light.get()(r, st.lambda).value.L;
                let rgb = fixel(in.film, st);
                let value = in.image.Load(pixel);
                in.image.Store(pixel, value + fv4(rgb * fixel.weight, fixel.weight));
                return;
            }
            let a_intr = a_opt.value;
            let s_intr = a_intr.intr_opt.value;
            let value = in.image.Load(pixel);
            in.image.Store(pixel, value + fv4(s_intr.p, fixel.weight));
        }

        let value = in.image.Load(pixel);
        in.image.Store(pixel, value / value.w);
    }
}
