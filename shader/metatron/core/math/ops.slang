implementing math;

namespace mtt {
    using namespace math;

    public fv4 operator*(Transform t, fv4 x) {
        return mul(t.transform, x);
    }

    public fv3 operator*(Transform t, fv3 x) {
        return normalize(mul(fv4(x, 0.f), t.inv_transform).xyz);
    }

    public Ray operator*(Transform t, Ray r) {
        var tr = r;
        tr.o = (t * fv4(r.o, 1.f)).xyz;
        tr.d = (t * fv4(r.d, 0.f)).xyz;
        return tr;
    }

    public Ray_Differential operator*(Transform t, Ray_Differential r) {
        var tr = r;
        tr.r = t * r.r;
        tr.rx = t * r.rx;
        tr.ry = t * r.ry;
        return tr;
    }

    public fv4 operator/(Transform t, fv4 x) {
        return mul(t.inv_transform, x);
    }

    public fv3 operator/(Transform t, fv3 x) {
        return normalize(mul(fv4(x, 0.f), t.transform).xyz);
    }

    public Ray operator/(Transform t, Ray r) {
        var tr = r;
        tr.o = (t / fv4(r.o, 1.f)).xyz;
        tr.d = (t / fv4(r.d, 0.f)).xyz;
        return tr;
    }

    public Ray_Differential operator/(Transform t, Ray_Differential r) {
        var tr = r;
        tr.r = t / r.r;
        tr.rx = t / r.rx;
        tr.ry = t / r.ry;
        return tr;
    }

    public Bounding_Box operator*(Transform t, Bounding_Box bbox) {
        var tbox: Bounding_Box;
        for (var i = 0; i < 8; ++i) {
            var p: fv3;
            for (var j = 0; j < 3; ++j) {
                let k = (i >> j) & 0x1;
                p[j] = k == 0 ? bbox.p_min[j] : bbox.p_max[j];
            }
            p = t * p;
            tbox.p_min = min(tbox.p_min, p);
            tbox.p_max = max(tbox.p_max, p);
        }
        return tbox;
    }

    public Bounding_Box operator/(Transform t, Bounding_Box bbox) {
        var tbox: Bounding_Box;
        for (var i = 0; i < 8; ++i) {
            var p: fv3;
            for (var j = 0; j < 3; ++j) {
                let k = (i >> j) & 0x1;
                p[j] = k == 0 ? bbox.p_min[j] : bbox.p_max[j];
            }
            p = t / p;
            tbox.p_min = min(tbox.p_min, p);
            tbox.p_max = max(tbox.p_max, p);
        }
        return tbox;
    }

    public Context operator*(Transform t, Context ctx) {
        var result = ctx;
        result.r = t * result.r;
        result.n = t * result.n;
        return result;
    }

    public Context operator/(Transform t, Context ctx) {
        var result = ctx;
        result.r = t / result.r;
        result.n = t / result.n;
        return result;
    }

    public Quaternion<T> operator*<T: __BuiltinFloatingPointType>(
        Quaternion<T> lhs, Quaternion<T> rhs
    ) {
        // Hamilton
        let x = lhs[0]; let y = lhs[1]; let z = lhs[2]; let w = lhs[3];
        let rx = rhs[0]; let ry = rhs[1]; let rz = rhs[2]; let rw = rhs[3];
        return {
            w * rx + x * rw + y * rz - z * ry,
            w * ry + y * rw + z * rx - x * rz,
            w * rz + z * rw + x * ry - y * rx,
            w * rw - x * rx - y * ry - z * rz,
        };
    }

    public Quaternion<T> operator*<T: __BuiltinFloatingPointType>(
        Quaternion<T> lhs, T rhs
    ) { return {lhs.data * rhs}; }

    public Quaternion<T> operator+<T: __BuiltinFloatingPointType>(
        Quaternion<T> lhs, Quaternion<T> rhs
    ) { return {lhs.data + rhs.data}; }

    public Quaternion<T> operator-<T: __BuiltinFloatingPointType>(
        Quaternion<T> x
    ) { return {-x.data}; }

    public Complex<T> operator+<T: __BuiltinFloatingPointType>(
        Complex<T> x, Complex<T> y
    ) {
        return {x.r + y.r, x.i + y.i};
    }

    public Complex<T> operator+=<T: __BuiltinFloatingPointType>(
        Complex<T> x, Complex<T> y
    ) { x = x + y; return x; }

    public Complex<T> operator+<T: __BuiltinFloatingPointType>(
        Complex<T> x, T y
    ) { return x + Complex<T>(y); }

    public Complex<T> operator+<T: __BuiltinFloatingPointType>(
        T x, Complex<T> y
    ) { return Complex<T>(x) + y; }

    public Complex<T> operator-<T: __BuiltinFloatingPointType>(
        Complex<T> x, Complex<T> y
    ) { return {x.r - y.r, x.i - y.i}; }

    public Complex<T> operator-=<T: __BuiltinFloatingPointType>(
        Complex<T> x, Complex<T> y
    ) { x = x - y; return x; }

    public Complex<T> operator-<T: __BuiltinFloatingPointType>(
        Complex<T> x, T y
    ) { return x - Complex<T>(y); }

    public Complex<T> operator-<T: __BuiltinFloatingPointType>(
        T x, Complex<T> y
    ) { return Complex<T>(x) - y; }

    public Complex<T> operator-<T: __BuiltinFloatingPointType>(
        Complex<T> x
    ) { return {-x.r, -x.i}; }

    public Complex<T> operator*<T: __BuiltinFloatingPointType>(
        Complex<T> x, Complex<T> y
    ) { return {
        x.r * y.r - x.i * y.i,
        x.r * y.i + x.i * y.r,
    }; }

    public Complex<T> operator*=<T: __BuiltinFloatingPointType>(
        Complex<T> x, Complex<T> y
    ) { x = x * y; return x; }

    public Complex<T> operator*<T: __BuiltinFloatingPointType>(
        Complex<T> x, T y
    ) { return x * Complex<T>(y); }

    public Complex<T> operator*<T: __BuiltinFloatingPointType>(
        T x, Complex<T> y
    ) { return Complex<T>(x) * y; }

    public Complex<T> operator/<T: __BuiltinFloatingPointType>(
        Complex<T> x, Complex<T> y
    ) {
        let denom = y.r * y.r + y.i * y.i;
        return {
            math::guarded_div(x.r * y.r + x.i * y.i, denom),
            math::guarded_div(x.i * y.r - x.r * y.i, denom),
        };
    }

    public Complex<T> operator/=<T: __BuiltinFloatingPointType>(
        Complex<T> x, Complex<T> y
    ) { x = x / y; return x; }

    public Complex<T> operator/<T: __BuiltinFloatingPointType>(
        Complex<T> x, T y
    ) { return x / Complex<T>(y); }

    public Complex<T> operator/<T: __BuiltinFloatingPointType>(
        T x, Complex<T> y
    ) { return Complex<T>(x) / y; }
}
