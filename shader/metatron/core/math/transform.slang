implementing math;

namespace mtt::math {
    public struct Transform {
        float4x4 transform;
        float4x4 inv_transform;
    }

    float4 operator*(Transform t, float4 x) {
        return mul(t.transform, x);
    }

    float3 operator*(Transform t, float3 x) {
        return normalize(mul(float4(x, 0), t.transform).xyz);
    }

    Ray operator*(Transform t, Ray r) {
        var tr = r;
        tr.Origin = (t * float4(r.Origin, 1)).xyz;
        tr.Direction = (t * float4(r.Direction, 0)).xyz;
        return tr;
    }

    Ray_Differential operator*(Transform t, Ray_Differential r) {
        var tr = r;
        tr.r = t * r.r;
        tr.rx = t * r.rx;
        tr.ry = t * r.ry;
        return tr;
    }

    float4 operator/(Transform t, float4 x) {
        return mul(t.inv_transform, x);
    }

    float3 operator/(Transform t, float3 x) {
        return normalize(mul(float4(x, 0), t.inv_transform).xyz);
    }

    Ray operator/(Transform t, Ray r) {
        var tr = r;
        tr.Origin = (t / float4(r.Origin, 1)).xyz;
        tr.Direction = (t / float4(r.Direction, 0)).xyz;
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
