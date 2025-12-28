module sampler;
__include sampler.halton;
__include sampler.independent;
__include sampler.proxy;
__include sampler.sobol;
import metatron.core;

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

    public interface Sampler {
        public func start(inout Context ctx) -> void;
        public func generate_1d(inout Context ctx) -> f32;
        public func generate_2d(inout Context ctx) -> fv2;
        public func generate_pixel_2d(inout Context ctx) -> fv2;
    }

    public struct Sampler_Tag: Sampler {
        tag<byte> idx;

        public func start(inout Context ctx) -> void {
            if (idx.type() == 0) return tag<Independent_Sampler>(idx).get().start(ctx);
            else if (idx.type() == 1) return tag<Halton_Sampler>(idx).get().start(ctx);
            else if (idx.type() == 2) return tag<Sobol_Sampler>(idx).get().start(ctx);
        }

        public func generate_1d(inout Context ctx) -> f32 {
            if (idx.type() == 0) return tag<Independent_Sampler>(idx).get().generate_1d(ctx);
            else if (idx.type() == 1) return tag<Halton_Sampler>(idx).get().generate_1d(ctx);
            else if (idx.type() == 2) return tag<Sobol_Sampler>(idx).get().generate_1d(ctx);
            else return {};
        }

        public func generate_2d(inout Context ctx) -> fv2 {
            if (idx.type() == 0) return tag<Independent_Sampler>(idx).get().generate_2d(ctx);
            else if (idx.type() == 1) return tag<Halton_Sampler>(idx).get().generate_2d(ctx);
            else if (idx.type() == 2) return tag<Sobol_Sampler>(idx).get().generate_2d(ctx);
            else return {};
        }

        public func generate_pixel_2d(inout Context ctx) -> fv2 {
            if (idx.type() == 0) return tag<Independent_Sampler>(idx).get().generate_pixel_2d(ctx);
            else if (idx.type() == 1) return tag<Halton_Sampler>(idx).get().generate_pixel_2d(ctx);
            else if (idx.type() == 2) return tag<Sobol_Sampler>(idx).get().generate_pixel_2d(ctx);
            else return {};
        }
    }
}
