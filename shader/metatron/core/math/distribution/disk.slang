implementing math;

namespace mtt::math {
    public struct Polar_Disk_Distribution {
        public func sample(fv2 u) -> fv2 {
            let r = sqrt(u[0]);
            let theta = 2.f * pi * u[1];
            return r * fv2(cos(theta), sin(theta));
        }

        public func pdf() -> f32 {
            return 1.f / pi;
        }
    }

    public struct Unifrom_Disk_Distribution {
        public func sample(fv2 u) -> fv2 {
            let disk_u = u * 2.f - 1.f;
            var r = 0.f;
            var theta = 0.f;

            if (all(abs(disk_u) < fv2(epsilon))) {
                return fv2(0.f);
            } else if (abs(disk_u[0]) > abs(disk_u[1])) {
                r = disk_u[0];
                theta = pi / 4.f * guarded_div(disk_u[1], disk_u[0]);
            } else {
                r = disk_u[1];
                theta = pi / 2.f - pi / 4.f * guarded_div(disk_u[0], disk_u[1]);
            }

            return r * fv2(cos(theta), sin(theta));
        }

        public func pdf() -> f32 {
            return 1.f / pi;
        }
    }
}
