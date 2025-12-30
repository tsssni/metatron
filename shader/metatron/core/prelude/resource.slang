implementing prelude;

namespace mtt {
    public static const u32 bindless_size = 8192;
    public struct Global {
        public Ptr<Ptr<byte>> vectors;
        public Ptr<Ptr<u32>> volumes;
        [vk_binding(1, 0)] public RaytracingAccelerationStructure accel;
        [vk_binding(2, 0)] public SamplerState sampler;
        [vk_binding(3, 0)] public SamplerState accessor;
        [vk_binding(4, 0)] public Texture2D[bindless_size] textures;
        [vk_binding(5, 0)] public Texture3D[bindless_size] grids;
    }
    [vk_binding(0, 0)] public ParameterBlock<Global> global;
}
