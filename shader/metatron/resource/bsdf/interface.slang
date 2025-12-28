implementing bsdf;

namespace mtt::bsdf {
    public struct Interface_Bsdf: Bsdf {
        u32 padding = 0u;

        public Optional<Interaction> operator()(fv3 wo, fv3 wi) {
            return Interaction(fv4(1.f), wo, 1.f);
        }

        public func sample(math::Context ctx, fv3 u) -> Optional<Interaction> {
            return Interaction(fv4(1.f), ctx.r.d, 1.f);
        }

        public func flags() -> Flags {
            return Flags::interface;
        }
    }
}
