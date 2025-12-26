implementing math;

namespace mtt::math {
    public struct Context {
        public Ray r = {};
        public fv3 n = {};
        public fv4 lambda = {};
        public bool inside;
    };
}
