implementing sampler;

namespace mtt::sampler {
    // TODO: support indpendent sampler
    public struct Independent_Sampler: Sampler {
        u64 rng = 0;
        u64 distr = 0;
        iv2 pixel = 0;
        i32 idx = 0;
        i32 dim = 0;

        [mutating] public func start(Context ctx) -> void {}
        [mutating] public func generate_1d() -> f32 { return 0; }
        [mutating] public func generate_2d() -> fv2 { return 0; }
        [mutating] public func generate_pixel_2d() -> fv2 { return 0; }
    }
}
