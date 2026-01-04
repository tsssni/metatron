import metatron;

namespace mtt {
    struct Constants {
        monte_carlo::Integrator_Tag integrator;
        accel::Acceleration_Tag accel;
        emitter::Emitter_Tag emitter;
        sampler::Sampler_Tag sampler;
        filter::Filter_Tag filter;
        photo::Lens_Tag lens;
        tag<photo::Film> film;
        u32 seed;
        uv2 range;
        math::Transform ct;
        buf<f32> fresnel;
        [vk_binding(1, 0)] RWTexture2D image;
    }
    [vk_binding(0, 0)] ParameterBlock<Constants> constants;

    [shader("compute"), numthreads(8, 8, 1)]
    func wave(
        iv3 global_idx: SV_DispatchThreadID,
    ) -> void {
        var value = fv4(0);
        let pixel = global_idx.xy;
        let cb = constants;
        let film = cb.film.get();
        var size: uv2;
        cb.image.GetDimensions(size.x, size.y);
        if (any(pixel != clamp(pixel, 0, size - 1))) return;
        bsdf::fresnel_reflectance_table = cb.fresnel;

        for (var n = cb.range.x; n < cb.range.y; ++n) {
            var sp: sampler::Proxy_Sampler;
            sp.ctx.pixel = pixel;
            sp.ctx.size = size;
            sp.ctx.idx = n;
            sp.ctx.spp = film.spp;
            sp.ctx.dim = 0;
            sp.ctx.seed = cb.seed;
            sp.sampler = cb.sampler;
            sp.start();

            let fixel = film(cb.filter, pixel, size, sp.generate_2d());
            let camera: photo::Camera;
            let s_opt = camera.sample(cb.lens, fixel.position, fixel.dxdy, sp.generate_2d());
            if (s_opt == none) continue;
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
            ctx.sample_index = n;
            ctx.max_depth = film.depth;

            let Li_opt = cb.integrator.sample(ctx);
            if (Li_opt == none) continue;
            var Li = Li_opt.value;
            Li.value /= s.pdf;
            let rgb = fixel(film, Li);
            value += fv4(rgb * fixel.weight, fixel.weight);
        }

        let sampled = cb.image.Load(pixel) + value;
        cb.image.Store(pixel, sampled);
    }
}
