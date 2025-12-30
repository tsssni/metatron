implementing math;

namespace mtt::math {
    public struct Exponential_Distribution {
        f32 a;
        public __init(f32 a) { this.a = a; }

        public func sample(f32 u) -> f32 {
            return -log(1.f - u) / a;
        }

        public func pdf(f32 x) -> f32 {
            return a * exp(-a * x);
        }
    }
}
