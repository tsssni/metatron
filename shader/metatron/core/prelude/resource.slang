implementing prelude;

namespace mtt {
    public struct Resources {
        public uptr vectors;
        public uptr volumes;
        [vk_binding(1, 1)] public RaytracingAccelerationStructure accel;
    }
    [vk_binding(0, 1)] public ParameterBlock<Resources> resources;

    public struct Textures {
        [vk_binding(0, 2)] public SamplerState sampler;
        [vk_binding(1, 2)] public Texture2D bindless[];
    }
    public ParameterBlock<Textures> textures;

    public struct Grids {
        [vk_binding(0, 3)] public Texture3D bindless[];
    }
    public ParameterBlock<Grids> grids;

}
