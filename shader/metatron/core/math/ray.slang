implementing math;

namespace mtt::math {
    public struct Ray {
        public fv3 o;
        public fv3 d;

        public func get() -> RayDesc {
            RayDesc r;
            r.Origin = o;
            r.Direction = d;
            r.TMin = epsilon;
            r.TMax = 65504.f;
            return r;
        }
    }

    public struct Ray_Differential {
        public Ray r;
        public Ray rx;
        public Ray ry;
        public bool differentiable;
    }
}
