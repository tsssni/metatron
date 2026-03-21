implementing math;

namespace mtt::math {
    public struct Complex<T: __BuiltinFloatingPointType> {
        public T r;
        public T i;
    };

    public func norm<T: __BuiltinFloatingPointType>(Complex<T> z) -> T {
        return z.r * z.r + z.i * z.i;
    }

    public func abs<T: __BuiltinFloatingPointType>(Complex<T> z) -> T {
        return math::pow<1,2>(norm(z));
    }

    public func sqrt<T: __BuiltinFloatingPointType>(Complex<T> z) -> Complex<T> {
        // z = a + bi, w = u + vi = sqrt(z)
        // u^2 - v^2 = a, 2uv = b
        let a = z.r;
        let b = z.i;

        if (b != T(0)) {
            // v = b / 2u
            // u^4 - a u^2 - b^2 / 4 = 0
            // u^2 = (a + sqrt(a^2 + b^2)) / 2
            let u = math::pow<1,2>((a + abs(z)) * T(0.5));
            let v = T(0.5) * b / u;
            return {u, v};
        } else if (a >= T(0)) {
            return {math::pow<1,2>(a), T(0)};
        } else {
            return {T(0), math::pow<1,2>(-a)};
        }
    }

    public func sqr<T: __BuiltinFloatingPointType>(
        Complex<T> z
    ) -> Complex<T> { return z * z; }

    public func pow<let n: u32, T: __BuiltinFloatingPointType>(Complex<T> x) -> Complex<T> {
        if (n == 0) return Complex<T>(T(1), T(0));
        if (n == 1) return x;
        if (n == 2) return x * x;
        return x * x * x;
    }

    public func pow<let n: u32, let d: u32, T: __BuiltinFloatingPointType>(Complex<T> x) -> Complex<T> {
        var y = x;
        if (d == 1) {}
        else if (d == 2) y = sqrt(x);
        return pow<n>(y);
    }

    public func guarded_div<T: __BuiltinFloatingPointType>(
        Complex<T> x, Complex<T> y
    ) -> Complex<T> { return x / y; }

    public func guarded_div<T: __BuiltinFloatingPointType>(
        T x, Complex<T> y
    ) -> Complex<T> { return x / y; }

    public func guarded_div<T: __BuiltinFloatingPointType>(
        Complex<T> x, T y
    ) -> Complex<T> { return x / y; }
}
