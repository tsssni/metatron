implementing sampler;

namespace mtt::sampler::proxy {
    public struct Sampler {
        public mtt::sampler::Context ctx;
        public mtt::sampler::Sampler sampler;

        [mutating] public func start() -> void { sampler.start(ctx); }
        [mutating] public func generate_1d() -> f32 { return sampler.generate_1d(ctx); }
        [mutating] public func generate_2d() -> fv2 { return sampler.generate_2d(ctx); }
        [mutating] public func generate_pixel_2d() -> fv2 { return sampler.generate_pixel_2d(ctx); }
    }
}
