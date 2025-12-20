import metatron;

namespace mtt {
    static const uint bindless_size = 8192;
    struct Global {
        Ptr<Ptr<byte>> vectors;
        RaytracingAccelerationStructure accel;
        SamplerState sampler;
        SamplerState accessor;
        Texture2D[bindless_size] textures;
        Texture3D[bindless_size] grids;
    }
    ParameterBlock<Global> global;

    struct Integrate {
        math::Transform transform;
        photo::Film film;
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
        let fixel = in.film(0, pixel, size, 0);

        let camera: photo::Camera;
        let s_opt = camera.sample(in.lens.get(), fixel.position, fixel.dxdy, fv2(0));
        if (s_opt == none) {
            in.image.Store(pixel, math::inf);
            return;
        }

        var s = s_opt.value;
        s.ray_differential = in.transform / s.ray_differential;
        s.default_differential = in.transform / s.default_differential;

        let r = s.ray_differential.r.get();
        var rq: RayQuery;
        rq.TraceRayInline(global.accel, 0, 0xff, r);
        while (rq.Proceed());
        if (rq.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
            in.image.Store(pixel, fv4(fv3(rq.CommittedRayT()), 1.0));
        else
            in.image.Store(pixel, fv4(0));
    }
}
