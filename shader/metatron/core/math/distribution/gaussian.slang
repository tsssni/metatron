implementing math;

namespace mtt::math {
    public struct Gaussian_Distribution {
        f32 mu = {};
        f32 sigma = {};

        public __init(f32 mu, f32 sigma) { this.mu = mu; this.sigma = sigma; }

        public func pdf(f32 x) -> f32 {
            return gaussian(x, mu, sigma);
        }
    };

    public struct Truncated_Gaussian_Distribution {
        f32 mu = {};
        f32 sigma = {};
        f32 cdf_a = {};
        f32 cdf_b = {};

        public __init() {}
        public __init(f32 mu, f32 sigma, f32 a, f32 b) {
            this.mu = mu; this.sigma = sigma;
            this.cdf_a = gaussian_cdf(a, mu, sigma);
            this.cdf_b = gaussian_cdf(b, mu, sigma);
        }

        public func sample(f32 u) -> f32 {
            var s = lerp(cdf_a, cdf_b, u);
            s = clamp(s, math::epsilon, 1.f - math::epsilon);
            return math::sqrt(2.f) * erfinv(2.f * s - 1.f) * sigma + mu;
        }

        public func pdf(f32 x) -> f32 {
            return math::guarded_div(gaussian(x, mu, sigma), (cdf_b - cdf_a));
        }
    }
}
