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

        for (var n = in.range.x; n < in.range.y; ++n) {
            sp.start({pixel, size, n, in.film.spp, 0, in.seed});
            let fixel = in.film(f, pixel, size, sp.generate_2d());
            let camera: photo::Camera;
            let s_opt = camera.sample(in.lens, fixel.position, fixel.dxdy, sp.generate_2d());
            if (s_opt == none) continue;
            var s = s_opt.value;
            s.ray_differential = in.ct / s.ray_differential;
            s.default_differential = in.ct / s.default_differential;

            let spec = spectra::Stochastic_Spectrum(sp.generate_1d());
            var ctx: monte_carlo::Context;
            ctx.accel = in.accel;
            ctx.emitter = in.emitter;
            ctx.sampler = sp;
            ctx.lambda = spec.lambda;
            ctx.ray_differential = s.ray_differential;
            ctx.default_differential = s.default_differential;
            ctx.render_to_camera = in.ct;
            ctx.pixel = pixel;
            ctx.sample_index = n;
            ctx.max_depth = in.film.depth;

            let Li_opt = in.integrator.sample(ctx);
            if (Li_opt == none) continue;
            var Li = Li_opt.value;
            Li.value /= s.pdf;
            let rgb = fixel(in.film, Li);
            in.image.Store(pixel, fv4(rgb * fixel.weight, fixel.weight));
        }

        in.image.Store(pixel, value / max(value.w, math::epsilon));
    }
}
