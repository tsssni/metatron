import metatron;

namespace mtt {
    struct Trace {
        Ptr<uint3> indices;

        Ptr<float3> vertices;
        Ptr<float3> normals;
        Ptr<float2> uvs;

        Ptr<float3> dpdu;
        Ptr<float3> dpdv;
        Ptr<float3> dndu;
        Ptr<float3> dndv;

        RWTexture2D film;
    }

    SamplerState sampler;
    SamplerState accessor;
    Texture2D textures[];
    Texture3D grids[];
    RaytracingAccelerationStructure accel;

    [shader("compute"), numthreads(8, 8, 1)]
    func trace(
        ParameterBlock<Trace> in,
    ) -> void {
        in.indices[0] = uint3(0, 1, 2);
        in.vertices[0] = float3(-1, -1, 0);
        in.vertices[1] = float3(-1,  2, 0);
        in.vertices[2] = float3( 2, -1, 0);

        var q = RayQuery<RAY_FLAG_NONE>();
        var r = RayDesc();
        r.Origin = float3(0,0,-1);
        r.Direction = float3(0,0,1);
        r.TMin = 0.0; r.TMax = float.maxValue;

        q.TraceRayInline(accel, RAY_FLAG_NONE, 0xff, r);
        while (q.Proceed()) {
            switch (q.CandidateType()) {
            case CANDIDATE_PROCEDURAL_PRIMITIVE:
                q.CommitProceduralPrimitiveHit(0.0); break;
            }
        }

        let t = textures[0].SampleLevel(sampler, float2(0.5), 0.0);
        let g = grids[0].SampleLevel(accessor, float3(0.5), 0.0);
        in.film.Store(0, t + g);
    }

    struct Postprocess {
        Texture2D film;
        RWTexture2D image;
    }

    [shader("compute"), numthreads(8, 8, 1)]
    func postprocess(
        int3 global_idx: SV_DispatchThreadID,
        ParameterBlock<Postprocess> in,
    ) -> void {
        var pixel = in.film.Load(global_idx);
        in.image.Store(global_idx.xy, pixel / pixel.w);
    }
}
