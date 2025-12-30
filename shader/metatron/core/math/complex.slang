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
        return math::sqrt(norm(z));
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
            let u = math::sqrt((a + abs(z)) * T(0.5));
            let v = T(0.5) * b / u;
            return {u, v};
        } else if (a >= T(0)) {
            return {math::sqrt(a), T(0)};
        } else {
            return {T(0), math::sqrt(-a)};
        }
    }

    public func sqr<T: __BuiltinFloatingPointType>(
        Complex<T> z
    ) -> Complex<T> { return z * z; }

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
