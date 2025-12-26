implementing math;

namespace mtt::math {
    public func cartesian_to_unit_spherical(fv3 d) -> fv2 {
        let theta = acos(d[1]);
        let phi = atan2(d[2], d[0]);
        return {theta, phi};
    }

    public func unit_spherical_to_cartesian(f32 cos_theta, f32 phi) -> fv3 {
        let sin_theta_2 = 1.f - cos_theta * cos_theta;
        let sin_theta = sqrt(sin_theta_2);
        let x = sin_theta * cos(phi);
        let z = sin_theta * sin(phi);
        let y = cos_theta;
        return {x, y, z};
    }

    public func unit_spherical_to_cartesian(fv2 s) -> fv3 {
        let theta = s[0];
        let phi = s[1];
        return unit_spherical_to_cartesian(cos(theta), phi);
    }

    public func cartesian_to_spherical(fv3 d) -> fv3 {
        let r = length(d);
        let nd = normalize(d);
        let s = cartesian_to_unit_spherical(nd);
        let theta = s[0];
        let phi = s[1];
        return {r, theta, phi};
    }

    public func spherical_to_cartesian(f32 r, f32 cos_theta, f32 phi) -> fv3 {
        let c = unit_spherical_to_cartesian(cos_theta, phi);
        return r * c;
    }

    public func spherical_to_cartesian(fv3 s) -> fv3 {
        let r = s[0];
        let theta = s[1];
        let phi = s[2];
        return spherical_to_cartesian(r, cos(theta), phi);
    }

    public func unit_to_cos_theta(fv3 d) -> f32 {
        return d[1];
    }

    public func unit_to_cos2_theta(fv3 d) -> f32 {
        return d[1] * d[1];
    }

    public func unit_to_sin2_theta(fv3 d) -> f32 {
        return max(0.f, 1.f - d[1] * d[1]);
    }

    public func unit_to_sin_theta(fv3 d) -> f32 {
        return sqrt(unit_to_sin2_theta(d));
    }

    public func unit_to_tan2_theta(fv3 d) -> f32 {
        return unit_to_sin2_theta(d) / unit_to_cos2_theta(d);
    }

    public func unit_to_tan_theta(fv3 d) -> f32 {
        return unit_to_sin_theta(d) / unit_to_cos_theta(d);
    }

    public func unit_to_cos_phi(fv3 d) -> f32 {
        let sin_theta = unit_to_sin_theta(d);
        return clamp(guarded_div(d[0], sin_theta), -1.f, 1.f);
    }

    public func unit_to_sin_phi(fv3 d) -> f32 {
        let sin_theta = unit_to_sin_theta(d);
        return clamp(guarded_div(d[2], sin_theta), -1.f, 1.f);
    }

}
