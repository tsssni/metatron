implementing math;

namespace mtt {
    public fv4 operator*(math::Transform t, fv4 x) {
        return mul(t.transform, x);
    }

    public fv3 operator*(math::Transform t, fv3 x) {
        return normalize(mul(fv4(x, 0), t.transform).xyz);
    }

    public math::Ray operator*(math::Transform t, math::Ray r) {
        var tr = r;
        tr.o = (t * fv4(r.o, 1)).xyz;
        tr.d = (t * fv4(r.d, 0)).xyz;
        return tr;
    }

    public math::Ray_Differential operator*(math::Transform t, math::Ray_Differential r) {
        var tr = r;
        tr.r = t * r.r;
        tr.rx = t * r.rx;
        tr.ry = t * r.ry;
        return tr;
    }

    public fv4 operator/(math::Transform t, fv4 x) {
        return mul(t.inv_transform, x);
    }

    public fv3 operator/(math::Transform t, fv3 x) {
        return normalize(mul(fv4(x, 0), t.inv_transform).xyz);
    }

    public math::Ray operator/(math::Transform t, math::Ray r) {
        var tr = r;
        tr.o = (t / fv4(r.o, 1)).xyz;
        tr.d = (t / fv4(r.d, 0)).xyz;
        return tr;
    }

    public math::Ray_Differential operator/(math::Transform t, math::Ray_Differential r) {
        var tr = r;
        tr.r = t / r.r;
        tr.rx = t / r.rx;
        tr.ry = t / r.ry;
        return tr;
    }

    public math::Bounding_Box operator|(math::Transform t, math::Bounding_Box bbox) {
        var tbox: math::Bounding_Box;
        for (var i = 0; i < 8; ++i) {
            var p: fv3;
            for (var j = 0; j < 3; ++j) {
                let k = (i >> j) & 0x1;
                p[j] = k == 0 ? bbox.p_min[j] : bbox.p_max[j];
            }
            p = t * p;
            tbox.p_min = min(tbox.p_min, p);
            tbox.p_max = max(tbox.p_max, p);
        }
        return tbox;
    }

    public math::Bounding_Box operator/(math::Transform t, math::Bounding_Box bbox) {
        var tbox: math::Bounding_Box;
        for (var i = 0; i < 8; ++i) {
            var p: fv3;
            for (var j = 0; j < 3; ++j) {
                let k = (i >> j) & 0x1;
                p[j] = k == 0 ? bbox.p_min[j] : bbox.p_max[j];
            }
            p = t / p;
            tbox.p_min = min(tbox.p_min, p);
            tbox.p_max = max(tbox.p_max, p);
        }
        return tbox;
    }

    public math::Context operator*(math::Transform t, math::Context ctx) {
        var result = ctx;
        result.r = t * result.r;
        result.n = t * result.n;
        return result;
    }

    public math::Context operator/(math::Transform t, math::Context ctx) {
        var result = ctx;
        result.r = t / result.r;
        result.n = t / result.n;
        return result;
    }
}
