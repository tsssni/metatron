implementing filter;

namespace mtt::filter {
    public struct Gaussian_Filter: Filter {
        math::Planar_Distribution piecewise;
        fv2 radius;
        f32 sigma;

        public f32 operator()(fv2 p) {
            let vx = math::gaussian(p[0], 0.f, sigma) - math::gaussian(radius[0], 0.f, sigma);
            let vy = math::gaussian(p[1], 0.f, sigma) - math::gaussian(radius[1], 0.f, sigma);
            return vx * vy;
        }

        public func sample(fv2 u) -> Optional<Interaction> {
            let p = piecewise.sample(u);
            let w = this(p.yx);
            let pdf = piecewise.pdf(p);
            return Interaction(p, w, pdf);
        }
    }
}
