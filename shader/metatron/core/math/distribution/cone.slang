implementing math;

namespace mtt::math {
    public struct Cone_Distribution {
        public __init(f32 cos_theta_max) { this.cos_theta_max = cos_theta_max; }

        public func sample(fv2 u) -> fv3 {
            let cos_theta = 1.f - u[0] * (1.f - cos_theta_max);
            let phi = u[1] * 2.f * pi;
            return unit_spherical_to_cartesian(cos_theta, phi);
        }

        public func pdf() -> f32 {
            return 1.f / (2.f * pi * (1.f - cos_theta_max));
        };

        private f32 cos_theta_max;
    };
}
