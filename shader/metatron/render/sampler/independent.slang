implementing sampler;

namespace mtt::sampler {
    // TODO: support indpendent sampler
    public struct Independent_Sampler: Sampler {
        public func start(inout Context ctx) -> void {}
        public func generate_1d(inout Context ctx) -> f32 { return 0; }
        public func generate_2d(inout Context ctx) -> fv2 { return 0; }
        public func generate_pixel_2d(inout Context ctx) -> fv2 { return 0; }
    }
}
