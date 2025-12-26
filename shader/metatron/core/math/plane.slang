implementing math;

namespace mtt::math {
    public struct Plane {
        f32 a, b, c, d;

        public __init(f32 a, f32 b, f32 c, f32 d) {
            this.a = a; this.b = b;
            this.c = c; this.d = d;
        }

        public __init(fv3 p, fv3 n) {
            a = n[0]; b = n[1]; c = n[2]; d = -dot(n, p);
        }
    }

    public func hit(Ray r, Plane p) -> Optional<f32> {
        let n = fv3(p.a, p.b, p.c);
        let no = dot(n, r.o) + p.d;
        let nd = dot(n, r.d);
        if (nd == 0.f && no != 0.f) return none;
        return -no / nd;
    }
}
