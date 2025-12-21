implementing light;

namespace mtt::light {
    public struct Area_Light: Light {
        shape::Shape_Tag shape;
        u32 primitive;

        public Optional<Interaction> operator()(
            math::Ray r, fv4 lambda
        ) {
            return {};
        }

        public func sample(math::Context ctx, fv2 u) -> Optional<Interaction> {
            let s_opt = shape.get().sample(ctx, u, primitive);
            if (s_opt == none) return none;
            let s_intr = s_opt.value;
            return Interaction(
                fv4(0.f),
                normalize(s_intr.p - ctx.r.o),
                s_intr.p,
                s_intr.t,
                s_intr.pdf,
            );
        }

        public func flags() -> Flags {
            return Flags(0);
        }
    }
}
