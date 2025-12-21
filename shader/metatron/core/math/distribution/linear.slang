implementing math;

namespace mtt::math {
    public struct Linear_Distribution {
        f32 a;
        f32 b;
        public __init(f32 a, f32 b) { this.a = a; this.b = b; }

        public func sample(f32 u) -> f32 {
            let a2 = a * a;
            let b2 = b * b;
            let x =  u * (a + b) / (a + sqrt(lerp(a2, b2, u)));
            return x;
        }

        public func pdf(f32 x) -> f32 {
            return 2.f * lerp(a, b, x) / (a + b);
        }
    }

    public struct Bilinear_Distribution {
        f32 a; f32 b; f32 c; f32 d;
        public __init(f32 a, f32 b, f32 c, f32 d) {
            this.a = a; this.b = b;
            this.c = c; this.d = d;
        }

        public func sample(fv2 u) -> fv2 {
            let x = Linear_Distribution(a + b, c + d).sample(u[0]);
            let y = Linear_Distribution(lerp(a, c, x), lerp(b, d, x)).sample(u[1]);
            return {x, y};
        }

        public func pdf(fv2 x) -> f32 {
            return 1.f
            * Linear_Distribution(a + b, c + d).pdf(x[0])
            * Linear_Distribution(lerp(a, c, x[0]), lerp(b, d, x[0])).pdf(x[1]);
        }
    }
}
