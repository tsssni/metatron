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
}
