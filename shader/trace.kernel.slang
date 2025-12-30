import metatron;

namespace mtt {
    struct Integrate {
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
        RWTexture2D image;
    }

    [shader("compute"), numthreads(8, 8, 1)]
    func integrate(
        iv3 global_idx: SV_DispatchThreadID,
        ParameterBlock<Integrate> in,
    ) -> void {
        var value = fv4(0);
        let pixel = global_idx.xy;
        let film = in.film.get();
        var size: uv2;
        in.image.GetDimensions(size.x, size.y);
        if (any(pixel != clamp(pixel, 0, size))) return;
        bsdf::fresnel_reflectance_table = in.fresnel;

        for (var n = in.range.x; n < in.range.y; ++n) {
            var sp: sampler::Proxy_Sampler;
            sp.ctx.pixel = pixel;
            sp.ctx.size = size;
            sp.ctx.idx = n;
            sp.ctx.spp = film.spp;
            sp.ctx.dim = 0;
            sp.ctx.seed = in.seed;
            sp.sampler = in.sampler;
            sp.start();

            let fixel = film(in.filter, pixel, size, sp.generate_2d());
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
            ctx.max_depth = film.depth;

            let Li_opt = in.integrator.sample(ctx);
            if (Li_opt == none) continue;
            var Li = Li_opt.value;
            Li.value /= s.pdf;
            let rgb = fixel(film, Li);
            value += fv4(rgb * fixel.weight, fixel.weight);
        }

        let sampled = in.image.Load(pixel) + value;
        in.image.Store(pixel, sampled);
    }

    struct Postprocess {
        RWTexture2D film;
        RWTexture2D image;
    }

    [shader("compute"), numthreads(8, 8, 1)]
    func postprocess(
        iv3 global_idx: SV_DispatchThreadID,
        ParameterBlock<Postprocess> in,
    ) -> void {
        let pixel = global_idx.xy;
        var size: uv2;
        in.image.GetDimensions(size.x, size.y);
        if (any(pixel != clamp(pixel, 0, size))) return;
        let film = in.film.Load(pixel);
        in.image.Store(pixel, film / max(film.w, math::epsilon));
    }
}
