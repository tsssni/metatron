import metatron;

namespace mtt {
    struct Image_Vector_Texture {
        uint tex;
    }
    static var image_vector_textures = Ptr<Image_Vector_Texture>(nullptr);

    struct Heterogeneous_Medium {
        uint grid;
    }
    static var heterogeneous_media = Ptr<Heterogeneous_Medium>(nullptr);

    struct Resources {
        Ptr<Image_Vector_Texture> image_vector_textures;
        Ptr<Heterogeneous_Medium> heterogeneous_media;
    }

    struct Integrate {
        uint2 range;
        uint spp;
        Ptr<Resources> ptrs;
        RWTexture2D film;
    }

    RaytracingAccelerationStructure accel;
    SamplerState sampler;
    SamplerState accessor;
    Texture2D textures[];
    Texture3D grids[];

    [shader("compute"), numthreads(8, 8, 1)]
    func integrate(
        int3 global_idx: SV_DispatchThreadID,
        ParameterBlock<Integrate> in,
    ) -> void {
        for (var i = in.range[0]; i < in.range[1]; ++i) {
            var q = RayQuery<RAY_FLAG_NONE>();
            var r = RayDesc();
            let x = (float(i) / in.spp) * 2 - 1;
            r.Origin = float3(0,0,0);
            r.Direction = normalize(float3(x, 0, 1));
            r.TMin = 0.0; r.TMax = float.maxValue;

            q.TraceRayInline(accel, RAY_FLAG_NONE, 0xff, r);
            while (q.Proceed()) {
                switch (q.CandidateType()) {
                case CANDIDATE_PROCEDURAL_PRIMITIVE:
                    q.CommitProceduralPrimitiveHit(0.0); break;
                }
            }
        }

        image_vector_textures = in.ptrs.image_vector_textures;
        heterogeneous_media = in.ptrs.heterogeneous_media;
        let t = textures[image_vector_textures[0].tex].SampleLevel(sampler, float2(0.5), 0.0);
        let g = grids[heterogeneous_media[0].grid].SampleLevel(accessor, float3(0.5), 0.0);
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
