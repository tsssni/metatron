module resource;
__exported import metatron.core;

namespace mtt {
    public static const u32 bindless_size = 8192;
    public struct Global {
        public Ptr<Ptr<byte>> vectors;
        public RaytracingAccelerationStructure accel;
        public SamplerState sampler;
        public SamplerState accessor;
        public Texture2D[bindless_size] textures;
        public Texture3D[bindless_size] grids;
    }
    public ParameterBlock<Global> global;
}
