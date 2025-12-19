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
        math::Transform ct;
        photo::Film film;
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
        in.image.Store(pixel, fv4(fixel.position, 0.0, 1.0));
    }
}
