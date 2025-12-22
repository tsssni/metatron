implementing sampler;

namespace mtt::sampler {
    // TODO: support indpendent sampler
    public struct Independent_Sampler: Sampler {
        u64 rng;
        u64 distr;
        iv2 pixel;
        i32 idx;
        i32 dim;

        [mutating] public func start(Context ctx) -> void {}
        [mutating] public func generate_1d() -> f32 { return 0; }
        [mutating] public func generate_2d() -> fv2 { return 0; }
        [mutating] public func generate_pixel_2d() -> fv2 { return 0; }
    }
}
