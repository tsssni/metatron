implementing math;

namespace mtt::math {
    public struct Bounding_Box {
        public fv3 p_min = high;
        public fv3 p_max = low;
    }

    public func inside(fv3 p, Bounding_Box bbox) -> bool {
        return all(p >= bbox.p_min) && all(p < bbox.p_max);
    }

    public func hit(
        Ray r,
        Bounding_Box bbox
    ) -> Optional<fv2> {
        var hit_min = (bbox.p_min - r.o) / r.d;
        var hit_max = (bbox.p_max - r.o) / r.d;
        for (var i = 0; i < 3; ++i)
            if (abs(r.d[i]) < epsilon) {
                hit_min[i] = -inf;
                hit_max[i] = +inf;
            } else if (hit_min[i] > hit_max[i]) {
                swap(hit_min[i], hit_max[i]);
            }

        let t_enter = max(hit_min);
        let t_exit = min(hit_max);
        if (t_exit < -epsilon || t_enter > t_exit + epsilon) return {};
        return fv2(t_enter, t_exit);
    }

    public func hitvi(
        Ray r,
        Bounding_Box bbox
    ) -> Optional<Tuple<f32, f32, u32, u32>> {
        var hit_min = (bbox.p_min - r.o) / r.d;
        var hit_max = (bbox.p_max - r.o) / r.d;
        for (var i = 0; i < 3; ++i)
            if (abs(r.d[i]) < epsilon) {
                hit_min[i] = -inf;
                hit_max[i] = +inf;
            } else if (hit_min[i] > hit_max[i]) {
                swap(hit_min[i], hit_max[i]);
            }

        let enter = maxvi(hit_min);
        let exit = minvi(hit_max);

        if (enter._0 < -epsilon || enter._0 > exit._0 + epsilon) return {};
        return Tuple<f32, f32, u32, u32>(enter._0, exit._0, enter._1, exit._1);
    }

    public func area(Bounding_Box bbox) -> f32 {
        if (any(bbox.p_min >= bbox.p_max)) return 0;
        let extent = bbox.p_max - bbox.p_min;
        return 2 * (extent[0] * extent[1] + extent[1] * extent[2] + extent[2] * extent[0]);
    }
}
