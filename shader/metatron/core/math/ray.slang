implementing math;

namespace mtt::math {
    public struct Ray {
        public fv3 o;
        public fv3 d;
    }

    public struct Ray_Differential {
        public Ray r;
        public Ray rx;
        public Ray ry;
        public bool differentiable;
    }
}
