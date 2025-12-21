implementing math;

namespace mtt::math {
    public func swap<T>(inout T x, inout T y) {
        let z = x;
        x = y;
        y = z;
    }

    public func guarded_div(f32 x, f32 y) -> f32 {
        return abs(y) < epsilon ? 0 : x / y;
    }
}
