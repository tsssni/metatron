implementing math;

namespace mtt::math {
    public struct Quaternion<T: __BuiltinFloatingPointType> {
        vector<T, 4> data;
        public __init(T x, T y, T z, T w) { data = {x, y, z, w}; }
        public __init(vector<T, 4> v) { data = v; }
        public __subscript(u32 i) -> T {
            get { return data[i]; }
            set { data[i] = newValue; }
        }

        public static Quaternion<T> from_axis_angle(vector<T, 3> axis, T angle) {
            let half_angle = angle * T(0.5);
            let sin_half = sin(half_angle);
            let cos_half = cos(half_angle);
            return Quaternion<T>(
                axis[0] * sin_half,
                axis[1] * sin_half,
                axis[2] * sin_half,
                cos_half,
            );
        }

        public static Quaternion<T> from_rotation_between(vector<T, 3> from, vector<T, 3> to) {
            var axis = cross(from, to);
            if (length(axis) < T(epsilon)) {
                let perp = abs(dot(from, vector<T, 3>(T(0), T(1), T(0)))) >= T(1.0 - 1e-6)
                ? vector<T, 3>(T(1), T(0), T(0))
                : vector<T, 3>(T(0), T(1), T(0));
                axis = cross(from, perp);
            }
            let rad = angle(from, to);
            return from_axis_angle(normalize(axis), rad);
        }

        public func get() -> vector<T, 4> {
            return data;
        }

        public func transform() -> matrix<T, 4, 4> {
            let x = data.x; let y = data.y;
            let z = data.z; let w = data.w;
            return {
                T(1.0)-T(2.0)*(y*y+z*z), T(2.0)*(x*y-z*w),        T(2.0)*(x*z+y*w),        T(0.0),
                T(2.0)*(x*y+z*w),        T(1.0)-T(2.0)*(x*x+z*z), T(2.0)*(y*z-x*w),        T(0.0),
                T(2.0)*(x*z-y*w),        T(2.0)*(y*z+x*w),        T(1.0)-T(2.0)*(x*x+y*y), T(0.0),
                T(0.0),                  T(0.0),                  T(0.0),                  T(1.0),
            };
        }
    }

    public func slerp<T: __BuiltinFloatingPointType>(
        Quaternion<T> q0, Quaternion<T> q1, T t
    ) -> Quaternion<T> {
        // quaternion fits x^2 + y^2 + z^2 + w^2 = 1, so it's 4D sphere which could use slerp

        let q0v = q0.data;
        let q1v = q1.data;
        var cos_theta = dot(q0v, q1v);

        // use lerp with small angle
        if (cos_theta > T(0.9995))
            return {normalize(lerp(q0v, q1v, t))};

        // ensure shortest path
        var q1v_adj = q1v;
        if (cos_theta < T(0)) {
            q1v_adj = -q1v_adj;
            cos_theta = -cos_theta;
        }

        let theta = angle(q0v, q1v_adj);
        let sin_theta = sin(theta);

        let scale1 = sin((T(1) - t) * theta) / sin_theta;
        let scale2 = sin(t * theta) / sin_theta;

        return {normalize(scale1 * q0v + scale2 * q1v_adj)};
    }

    public func conj<T: __BuiltinFloatingPointType>(
        Quaternion<T> q
    ) -> Quaternion<T> {
        return {-q[0], -q[1], -q[2], q[3]};
    }

    public func rotate<T: __BuiltinFloatingPointType>(
        vector<T, 4> x, Quaternion<T> q
    ) -> vector<T, 4> {
        let p = Quaternion<T>(x[0], x[1], x[2], T(0));
        let r = q * p * conj(q);
        return {r[0], r[1], r[2], T(0)};
    }
}
