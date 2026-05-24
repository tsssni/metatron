module sampler;
__include sampler.halton;
__include sampler.heitz;
__include sampler.independent;
__include sampler.proxy;
__include sampler.z_sobol;
import metatron.core;

#define DISPATCH_V(method, ...) \
         if (idx.type() == 0) return tag<Independent_Sampler>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 1) return tag<Halton_Sampler>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 2) return tag<Z_Sobol_Sampler>(idx).get().method(__VA_ARGS__); \
    else if (idx.type() == 3) return tag<Heitz_Sampler>(idx).get().method(__VA_ARGS__);

#define DISPATCH_R(method, ...) DISPATCH_V(method, __VA_ARGS__) else return {};

namespace mtt::sampler {
    public struct Context {
        public uv4 data;
        public uv2 pixel;
        public uv2 size;
        public u32 idx;
        public u32 spp;
        public u32 dim;
        public u32 seed;
    }

    public struct Sampler {
        tag<byte> idx;

        public func start(inout Context ctx) -> void { DISPATCH_V(start, ctx) }
        public func generate_1d(inout Context ctx) -> f32 { DISPATCH_R(generate_1d, ctx) }
        public func generate_2d(inout Context ctx) -> fv2 { DISPATCH_R(generate_2d, ctx) }
        public func generate_pixel_2d(inout Context ctx) -> fv2 { DISPATCH_R(generate_pixel_2d, ctx) }
    }
}

#undef DISPATCH_V
#undef DISPATCH_R
