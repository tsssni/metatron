implementing sampler;

namespace mtt::sampler {
    public struct Proxy_Sampler {
        public Context ctx;
        public Sampler_Tag sampler;

        [mutating] public func start() -> void { sampler.start(ctx); }
        [mutating] public func generate_1d() -> f32 { return sampler.generate_1d(ctx); }
        [mutating] public func generate_2d() -> fv2 { return sampler.generate_2d(ctx); }
        [mutating] public func generate_pixel_2d() -> fv2 { return sampler.generate_pixel_2d(ctx); }
    }
}
