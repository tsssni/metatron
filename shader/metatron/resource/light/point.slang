implementing light;

namespace mtt::light {
    public struct Point_Light: Light {
        spectra::Spectrum_Tag L;

        public Optional<Interaction> operator()(
            math::Ray r, fv4 lambda
        ) { return {}; }

        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> {
            let wi = normalize(-ctx.r.o);
            let r = length(ctx.r.o);
            return Interaction(
                (ctx.lambda & L.get()) / (r * r),
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
