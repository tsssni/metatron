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

    public func cbrt<T: __BuiltinFloatingPointType>(T x) -> T {
        return pow(x, T(1.0) / T(3.0));
    }

    public func sqrt<T: __BuiltinFloatingPointType>(T x) -> T {
        return sqrt(max(T(0), x));
    }

    public func sqr<T: IArithmetic>(T x) -> T {
        return x * x;
    }

    public func pow<let n: u32, T: IArithmetic>(T x) -> T {
        if (n == 0) return T(1);
        if (n == 1) return x;
        if (n == 2) return x * x;
        if (n == 3) return x * x * x;

        var result = T(1);
        var base = x;
        var e = n;
        while (e > 0) {
            if ((e & 1) != 0) result = result * base;
            base = base * base;
            e >>= 1;
        }
        return result;
    }

    public func pow<let n: u32, let d: u32, T: __BuiltinFloatingPointType>(T x) -> T {
        var y = x;
        if (d == 1) {}
        else if (d == 2) y = sqrt(x);
        else if (d == 3) y = cbrt(x);
        else y = pow(x, T(1.0) / T(d));
        return pow<n>(y);
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
