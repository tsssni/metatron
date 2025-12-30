implementing math;

namespace mtt::math {
    public func sinc(f32 x) -> f32 {
        return guarded_div(sin(pi * x), pi * x);
    };

    public func windowed_sinc(f32 x, f32 tau) -> f32 {
        return sinc(x) * sinc(x / tau);
    }

    public func radians(f32 degree) -> f32 {
        return degree / 180.f * math::pi;
    }

    public func degrees(f32 radian) -> f32 {
        return radian / math::pi * 180.f;
    }
}
