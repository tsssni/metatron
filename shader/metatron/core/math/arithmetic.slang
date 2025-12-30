implementing math;

namespace mtt::math {
    public func swap<T>(inout T x, inout T y) {
        let z = x;
        x = y;
        y = z;
    }

    public func guarded_div<T: __BuiltinFloatingPointType>(T x, T y) -> T {
        return abs(y) < T(epsilon) ? T(0) : x / y;
    }

    public func pow<T: __BuiltinIntegerType>(T x, T n) -> T {
        var y = T(1);
        while (n != T(0)) {
            if ((n & T(1)) != T(0))
                y = y * x;
            x = x * x;
            n >>= 1;
        }
        return y;
    }

    public func sqrt<T: __BuiltinFloatingPointType>(T x) -> T {
        return sqrt(max(T(0), x));
    }

    public func sqr<T: IArithmetic>(T x) -> T {
        return x * x;
    }

    public func log2i(u32 x) -> u32 {
        return firstbithigh(x);
    }

    public func acos<T: __BuiltinFloatingPointType>(T x) -> T {
        return acos(clamp(x, T(-1), T(1)));
    }

    public func asin<T: __BuiltinFloatingPointType>(T x) -> T {
        return asin(clamp(x, T(-1), T(1)));
    }

    public func atan2<T: __BuiltinFloatingPointType>(T y, T x) -> T {
        var z = atan2(y, x);
        if (z < T(0))
            z += T(2) * T(math::pi);
        return z;
    }
}
