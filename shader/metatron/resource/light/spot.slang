implementing light;

namespace mtt::light {
    public struct Spot_Light: Light {
        spectra::Spectrum_Tag L;
        f32 falloff_start_cos_theta;
        f32 falloff_end_cos_theta;

        public Optional<Interaction> operator()(
            math::Ray r, fv4 lambda
        ) { return {}; }

        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> {

            let d = fv3(0.f, 0.f, 1.f);
            let wi = normalize(-ctx.r.o);
            let r = length(ctx.r.o);

            let cos_theta = dot(d, -wi);
            let intensity = smoothstep(
                falloff_end_cos_theta,
                falloff_start_cos_theta,
                cos_theta
            );

            return Interaction(
                (ctx.lambda & L) * intensity / (r * r),
                wi,
                {0.f},
                r,
                1.f,
            );
        }

        public func flags() -> Flags {
            return Flags::delta;
        }
    }
}
