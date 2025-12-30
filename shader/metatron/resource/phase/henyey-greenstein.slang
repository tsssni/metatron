implementing phase;

namespace mtt::phase {
    public struct Henyey_Greenstein_Phase_Function: Phase_Function {
        f32 g = 0.f;
        public __init() {}
        public __init(f32 g) { this.g = g; }

        public Optional<Interaction> operator()(fv3 wo, fv3 wi) {
            let f = math::guarded_div(
                (1.f - g * g) / (4.f * math::pi),
                pow(1.f + g * g + 2.f * g * dot(-wo, wi), 1.5f)
            );
            return Interaction(fv4(f), wi, f);
        }

        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> {
            let cos_theta = abs(g) < math::epsilon ? 1.f - 2.f * u[0]
            : -1.f / (2.f * g) * (1.f + g * g - pow(math::guarded_div(1.f - g * g, 1.f + g - 2.f * g * u[0]), 2.f));
            let phi = 2.f * math::pi * u[1];

            let wi = math::unit_spherical_to_cartesian(cos_theta, phi);
            let x = this({0.f, -1.f, 0.f}, wi);
            return x;
        }
    }
}
