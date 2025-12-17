import metatron;

namespace mtt {
    static const uint bindless_size = 8192;
    struct Global {
        Ptr<Ptr<uint8_t>> vectors;
        SamplerState sampler;
        Texture2D[bindless_size] textures;
    }
    ParameterBlock<Global> global;

    struct Integrate {
        uint idx;
        uint offset;
        RWTexture2D film;
    }

    [shader("compute"), numthreads(8, 8, 1)]
    func integrate(
        int3 global_idx: SV_DispatchThreadID,
        ParameterBlock<Integrate> in,
    ) -> void {
        var size = int2(0);
        in.film.GetDimensions(size.x, size.y);
        if (any(global_idx.xy != clamp(global_idx.xy, 0, size.xy))) return;

        let uv = (float2(global_idx.xy) + 0.5) / size.xy;
        let tid = *Ptr<uint>(global.vectors[in.idx] + in.offset) & 0xfffffu;
        let color = global.textures[tid].SampleGrad(global.sampler, uv, 0.0, 0.0);
        in.film.Store(global_idx.xy, color);
    }
}
