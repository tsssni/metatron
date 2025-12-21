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

    public func sqrt<T: __BuiltinFloatingPointType>(T x) -> T {
        return sqrt(max(T(0), x));
    }

    public func sqr<T: IArithmetic>(T x) -> T {
        return x * x;
    }
}
