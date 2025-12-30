implementing math;

namespace mtt::math {
    public struct Sphere_Distribution {
        public func sample(fv2 u) -> fv3 {
            let cos_theta = 1.f - 2.f * u[0];
            let sin_theta = sqrt(1 - cos_theta * cos_theta);
            let phi = 2.f * pi * u[1];
            return {sin_theta * cos(phi), cos_theta, sin_theta * sin(phi)};
        }

        public func pdf() -> f32 {
            return 1.f / (4.f * pi);
        }
    };

    public struct Hemisphere_Distribution {
        public func sample(fv2 u) -> fv3 {
            let z = u[0];
            let r = sqrt(1 - z * z);
            let phi = 2.f * pi * u[1];
            return {r * cos(phi), r * sin(phi), z};
        }

        public func pdf() -> f32 {
            return 1.f / (2.f * pi);
        }
    };

    public struct Cosine_Hemisphere_Distribution {
        public func sample(fv2 u) -> fv3 {
            let distr: Unifrom_Disk_Distribution;
            let d = distr.sample(u);
            return {d[0], sqrt(1.f - sqr(d[0]) - sqr(d[1])), d[1]};
        }

        public func pdf(f32 cos_theta) -> f32 {
            return cos_theta / pi;
        }
    };
}
