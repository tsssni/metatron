implementing filter;

namespace mtt::filter {
    public struct Lanczos_Filter: Filter {
        math::Planar_Distribution distribution;
        fv2 radius;
        f32 tau;

        public f32 operator()(fv2 p) {
            var v: fv2;
            for (var i = 0; i < 2; ++i)
                v[i] = abs(p[i]) > radius[i] ? 0.f : math::windowed_sinc(p[i], tau);
            return math::prod(v);
        }

        public func sample(fv2 u) -> Optional<Interaction> {
            let p = distribution.sample(u);
            let w = this(p.yx);
            let pdf = distribution.pdf(p);
            return Interaction(p, w, pdf);
        }
    }
}
