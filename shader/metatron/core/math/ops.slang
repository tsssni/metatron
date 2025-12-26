implementing math;

namespace mtt {
    using namespace math;

    public fv4 operator*(Transform t, fv4 x) {
        return mul(t.transform, x);
    }

    public fv3 operator*(Transform t, fv3 x) {
        return normalize(mul(fv4(x, 0.f), t.transform).xyz);
    }

    public Ray operator*(Transform t, Ray r) {
        var tr = r;
        tr.o = (t * fv4(r.o, 1.f)).xyz;
        tr.d = (t * fv4(r.d, 0.f)).xyz;
        return tr;
    }

    public Ray_Differential operator*(Transform t, Ray_Differential r) {
        var tr = r;
        tr.r = t * r.r;
        tr.rx = t * r.rx;
        tr.ry = t * r.ry;
        return tr;
    }

    public fv4 operator/(Transform t, fv4 x) {
        return mul(t.inv_transform, x);
    }

    public fv3 operator/(Transform t, fv3 x) {
        return normalize(mul(fv4(x, 0.f), t.inv_transform).xyz);
    }

    public Ray operator/(Transform t, Ray r) {
        var tr = r;
        tr.o = (t / fv4(r.o, 1.f)).xyz;
        tr.d = (t / fv4(r.d, 0.f)).xyz;
        return tr;
    }

    public Ray_Differential operator/(Transform t, Ray_Differential r) {
        var tr = r;
        tr.r = t / r.r;
        tr.rx = t / r.rx;
        tr.ry = t / r.ry;
        return tr;
    }

    public Bounding_Box operator*(Transform t, Bounding_Box bbox) {
        var tbox: Bounding_Box;
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

    public Bounding_Box operator/(Transform t, Bounding_Box bbox) {
        var tbox: Bounding_Box;
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

    public Context operator*(Transform t, Context ctx) {
        var result = ctx;
        result.r = t * result.r;
        result.n = t * result.n;
        return result;
    }

    public Context operator/(Transform t, Context ctx) {
        var result = ctx;
        result.r = t / result.r;
        result.n = t / result.n;
        return result;
    }
}
