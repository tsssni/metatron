implementing media;

namespace mtt::media {
    public struct Vaccum_Medium: Medium {
        u32 padding;

        public func sample(math::Context ctx, f32 t_max, f32 u) -> Optional<Interaction> {
            return Interaction(
                phase::Phase_Function_Union(),
                ctx.r.o + t_max * ctx.r.d,
                t_max, fv4(1.f),
                {}, {}, {}, {}, {},
            );
        }
    }
}
