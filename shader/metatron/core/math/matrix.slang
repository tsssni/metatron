implementing math;

namespace mtt::math {
    public func inverse<T: __BuiltinFloatingPointType>(
        matrix<T, 2, 2> m
    ) -> matrix<T, 2, 2> {
        let det = m[0][0] * m[1][1] - m[0][1] * m[1][0];
        let inv_det = math::guarded_div(T(1), det);

        var result: matrix<T, 2, 2>;
        result[0][0] = m[1][1] * inv_det;
        result[0][1] = -m[0][1] * inv_det;
        result[1][0] = -m[1][0] * inv_det;
        result[1][1] = m[0][0] * inv_det;
        return result;
    }

    public func inverse<T: __BuiltinFloatingPointType>(
        matrix<T, 3, 3> m
    ) -> matrix<T, 3, 3> {
        let c00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
        let c01 = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]);
        let c02 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

        let c10 = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]);
        let c11 = m[0][0] * m[2][2] - m[0][2] * m[2][0];
        let c12 = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]);

        let c20 = m[0][1] * m[1][2] - m[0][2] * m[1][1];
        let c21 = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]);
        let c22 = m[0][0] * m[1][1] - m[0][1] * m[1][0];

        let det = m[0][0] * c00 + m[0][1] * c01 + m[0][2] * c02;
        let inv_det = math::guarded_div(T(1), det);

        var result: matrix<T, 3, 3>;
        result[0][0] = c00 * inv_det;
        result[0][1] = c10 * inv_det;
        result[0][2] = c20 * inv_det;
        result[1][0] = c01 * inv_det;
        result[1][1] = c11 * inv_det;
        result[1][2] = c21 * inv_det;
        result[2][0] = c02 * inv_det;
        result[2][1] = c12 * inv_det;
        result[2][2] = c22 * inv_det;
        return result;
    }

    public func inverse<T: __BuiltinFloatingPointType>(
        matrix<T, 4, 4> m
    ) -> matrix<T, 4, 4> {
        let s0 = m[0][0] * m[1][1] - m[1][0] * m[0][1];
        let s1 = m[0][0] * m[1][2] - m[1][0] * m[0][2];
        let s2 = m[0][0] * m[1][3] - m[1][0] * m[0][3];
        let s3 = m[0][1] * m[1][2] - m[1][1] * m[0][2];
        let s4 = m[0][1] * m[1][3] - m[1][1] * m[0][3];
        let s5 = m[0][2] * m[1][3] - m[1][2] * m[0][3];

        let c5 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
        let c4 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
        let c3 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
        let c2 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
        let c1 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
        let c0 = m[2][0] * m[3][1] - m[3][0] * m[2][1];

        let det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
        let inv_det = math::guarded_div(T(1), det);

        var result: matrix<T, 4, 4>;
        result[0][0] = (m[1][1] * c5 - m[1][2] * c4 + m[1][3] * c3) * inv_det;
        result[0][1] = (-m[0][1] * c5 + m[0][2] * c4 - m[0][3] * c3) * inv_det;
        result[0][2] = (m[3][1] * s5 - m[3][2] * s4 + m[3][3] * s3) * inv_det;
        result[0][3] = (-m[2][1] * s5 + m[2][2] * s4 - m[2][3] * s3) * inv_det;

        result[1][0] = (-m[1][0] * c5 + m[1][2] * c2 - m[1][3] * c1) * inv_det;
        result[1][1] = (m[0][0] * c5 - m[0][2] * c2 + m[0][3] * c1) * inv_det;
        result[1][2] = (-m[3][0] * s5 + m[3][2] * s2 - m[3][3] * s1) * inv_det;
        result[1][3] = (m[2][0] * s5 - m[2][2] * s2 + m[2][3] * s1) * inv_det;

        result[2][0] = (m[1][0] * c4 - m[1][1] * c2 + m[1][3] * c0) * inv_det;
        result[2][1] = (-m[0][0] * c4 + m[0][1] * c2 - m[0][3] * c0) * inv_det;
        result[2][2] = (m[3][0] * s4 - m[3][1] * s2 + m[3][3] * s0) * inv_det;
        result[2][3] = (-m[2][0] * s4 + m[2][1] * s2 - m[2][3] * s0) * inv_det;

        result[3][0] = (-m[1][0] * c3 + m[1][1] * c1 - m[1][2] * c0) * inv_det;
        result[3][1] = (m[0][0] * c3 - m[0][1] * c1 + m[0][2] * c0) * inv_det;
        result[3][2] = (-m[3][0] * s3 + m[3][1] * s1 - m[3][2] * s0) * inv_det;
        result[3][3] = (m[2][0] * s3 - m[2][1] * s1 + m[2][2] * s0) * inv_det;

        return result;
    }

    public func least_squares<T: __BuiltinFloatingPointType, let h: u32>(
        matrix<T, h, 2> a, vector<T, h> b
    ) -> vector<T, 2> {
        let a_t = transpose(a);
        return mul(inverse(mul(a_t, a)), mul(a_t, b));
    }

    public func least_squares<T: __BuiltinFloatingPointType, let h: u32>(
        matrix<T, h, 3> a, vector<T, h> b
    ) -> vector<T, 3> {
        let a_t = transpose(a);
        return mul(inverse(mul(a_t, a)), mul(a_t, b));
    }

    public func least_squares<T: __BuiltinFloatingPointType, let h: u32>(
        matrix<T, h, 4> a, vector<T, h> b
    ) -> vector<T, 4> {
        let a_t = transpose(a);
        return mul(inverse(mul(a_t, a)), mul(a_t, b));
    }
}
