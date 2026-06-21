import metatron;

namespace mtt {
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
        u32 integrator;
        [vk_binding(1, 0)] RWTexture2D image;
    }
    [vk_binding(0, 0)] ParameterBlock<Constants> constants;

    [shader("compute"), numthreads(8, 8, 1)]
    func wave(
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

        let intg = tag<monte_carlo::Radiative_Integrator>(cb.integrator).get();
        let Li_opt = intg.sample(ctx);
        if (Li_opt == none) return;
        var Li = Li_opt.value;
        Li.value /= s.pdf;
        let rgb = fixel(film, Li);
        let value = fv4(rgb * fixel.weight, fixel.weight);

        let sampled = cb.image.Load(pixel) + value;
        cb.image.Store(pixel, sampled);
    }
}
