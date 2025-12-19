implementing math;

namespace mtt::math {
    public typedef RayDesc Ray;
    public struct Ray_Differential {
        Ray r;
        Ray rx;
        Ray ry;
        bool differentiable;
    }
}
