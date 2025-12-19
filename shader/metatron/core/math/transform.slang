implementing math;

namespace mtt::math {
    public struct Transform {
        fm4 transform;
        fm4 inv_transform;
    }

    fv4 operator*(Transform t, fv4 x) {
        return mul(t.transform, x);
    }

    fv3 operator*(Transform t, fv3 x) {
        return normalize(mul(fv4(x, 0), t.transform).xyz);
    }

    Ray operator*(Transform t, Ray r) {
        var tr = r;
        tr.Origin = (t * fv4(r.Origin, 1)).xyz;
        tr.Direction = (t * fv4(r.Direction, 0)).xyz;
        return tr;
    }

    Ray_Differential operator*(Transform t, Ray_Differential r) {
        var tr = r;
        tr.r = t * r.r;
        tr.rx = t * r.rx;
        tr.ry = t * r.ry;
        return tr;
    }

    fv4 operator/(Transform t, fv4 x) {
        return mul(t.inv_transform, x);
    }

    fv3 operator/(Transform t, fv3 x) {
        return normalize(mul(fv4(x, 0), t.inv_transform).xyz);
    }

    Ray operator/(Transform t, Ray r) {
        var tr = r;
        tr.Origin = (t / fv4(r.Origin, 1)).xyz;
        tr.Direction = (t / fv4(r.Direction, 0)).xyz;
        return tr;
    }

    Ray_Differential operator/(Transform t, Ray_Differential r) {
        var tr = r;
        tr.r = t / r.r;
        tr.rx = t / r.rx;
        tr.ry = t / r.ry;
        return tr;
    }
}
