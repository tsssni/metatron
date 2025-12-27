implementing media;

namespace mtt::media {
    public struct Vaccum_Medium: Medium {
        public func sample(math::Context ctx, f32 t_max, f32 u) -> Optional<Interaction> {
            return Interaction(
                ctx.r.o + t_max * ctx.r.d,
                phase::Phase_Function_Tag(),
                t_max, fv4(1.f),
                {}, {}, {}, {}, {},
            );
        }
    }
}
