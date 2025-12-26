implementing math;

namespace mtt::math {
    public struct Spectrum_Distribution {
        public func sample(f32 u) -> f32 {
            return 538.f - 138.888889f * atanh(0.85691062f - 1.82750197f * u);
        }

        public func pdf(f32 x) -> f32 {
            if (x < 360.f || x > 830.f) return 0.f;
            return 0.0039398042f / math::sqr(cosh(0.0072f * (x - 538.f)));
        }
    }
}
