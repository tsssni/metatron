implementing math;

namespace mtt::math {
    public func guarded_div(f32 x, f32 y) -> f32 {
        return abs(y) < epsilon ? 0 : x / y;
    }
}
