implementing light;

namespace mtt::light {
    public struct Environment_Light: Light {
        texture::Spectrum_Texture_Tag env_map;

        public Optional<Interaction> operator()(
            math::Ray r, fv4 lambda
        ) {
            let s = math::cartesian_to_spherical(r.d);
            let radius = s.x; let theta = s.y; let phi = s.z;
            let u = 1.f - phi / (2.f * math::pi);
            let v = theta / math::pi;
            let t = env_map({{u, v}}, lambda);
            let J = 2.f * math::sqr(math::pi) * sin(theta);
            return Interaction(
                t,
                r.d,
                r.o + r.d * 65504.f,
                65504.f,
                math::guarded_div(env_map.pdf({u, v}), J),
            );
        }

        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> {
            let uv = env_map.sample(ctx, u);
            let phi = (1.f - uv[0]) * 2.f * math::pi;
            let theta = uv[1] * math::pi;
            let wi = math::unit_spherical_to_cartesian({theta, phi});
            return this({ctx.r.o, wi}, ctx.lambda);
        }

        public func flags() -> Flags {
            return Flags::inf;
        }
    }
}
