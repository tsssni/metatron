module sampler;
__include sampler.halton;
__include sampler.independent;
__include sampler.sobol;
import metatron.core;

namespace mtt::sampler {
    public struct Context {
        public uv2 pixel;
        public uv2 size;
        public u32 idx;
        public u32 spp;
        public u32 dim;
        public u32 seed;
    }

    public interface Sampler {
        [mutating] public func start(Context ctx) -> void;
        [mutating] public func generate_1d() -> f32;
        [mutating] public func generate_2d() -> fv2;
        [mutating] public func generate_pixel_2d() -> fv2;
    }

    public struct Sampler_Union {
        Independent_Sampler independent;
        Halton_Sampler halton;
        Sobol_Sampler sobol;
        u32 idx;

        public func get() -> Sampler {
            if (idx == 0) return independent;
            else if (idx == 1) return halton;
            else if (idx == 2) return sobol;
            else return {};
        }
    }
}
