import metatron;

namespace mtt {
    struct Input {
        Ptr<uint3> indices;

        Ptr<float3> vertices;
        Ptr<float3> normals;
        Ptr<float2> uvs;

        Ptr<float3> dpdu;
        Ptr<float3> dpdv;
        Ptr<float3> dndu;
        Ptr<float3> dndv;
    }

    ConstantBuffer<Input> in;
    RWTexture2D film;
    SamplerState sampler;
    SamplerState accessor;
    Texture2D textures[];
    Texture3D grids[];

    [shader("compute"), numthreads(8, 8, 1)]
    func trace() -> void {
        var t = textures[0].Sample(sampler, float2(0.5));
        var g = grids[0].Sample(accessor, float3(0.5));
        in.indices[0] = uint3(0);
        film.Store(0, t + g);
    }
}
