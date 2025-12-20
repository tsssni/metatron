import metatron;

namespace mtt {
    static const uint bindless_size = 8192;
    struct Global {
        Ptr<Ptr<byte>> vectors;
        RaytracingAccelerationStructure accel;
        SamplerState sampler;
        Texture2D[bindless_size] textures;
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
        uv2 pixel = global_idx.xy;
        uv2 size;
        in.image.GetDimensions(size.x, size.y);
        let fixel = in.film(0, pixel, size, 0);

        let camera: photo::Camera;
        let s_opt = camera.sample(in.lens.get(), fixel.position, fixel.dxdy, fv2(0));
        if (s_opt == none) return;
        var s = s_opt.value;
        s.ray_differential = in.transform / s.ray_differential;
        s.default_differential = in.transform / s.default_differential;
        in.image.Store(pixel, fv4(s.ray_differential.r.d, 1.0));
    }
}
