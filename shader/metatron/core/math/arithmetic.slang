implementing math;

namespace mtt::math {
    public func guarded_div(float x, float y) -> float {
        return abs(y) < epsilon ? 0 : x / y;
    }
}
