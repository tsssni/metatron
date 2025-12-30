implementing light;

namespace mtt::light {
    public struct Parallel_Light: Light {
        spectra::Spectrum_Tag L;

        public Optional<Interaction> operator()(
            math::Ray r, fv4 lambda
        ) { return {}; }

        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> {
            let wi = fv3(0.f, 0.f, -1.f);
            return Interaction(
                ctx.lambda & L,
                wi,
                ctx.r.o + 65504.f * wi,
                65504.f,
                1.f,
            );
        }

        public func flags() -> Flags {
            return Flags(Flags::delta & Flags::inf);
        }
    }
}
