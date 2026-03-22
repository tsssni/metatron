implementing media;

namespace mtt::media {
    public struct Vaccum_Medium {
        u32 padding;

        public struct Iterator {
            math::Ray r = {};
            f32 t_max = 0.f;

            __init() {}
            __init(math::Ray r, f32 t_max) {
                this.r = r;
                this.t_max = t_max;
            }

            [mutating] public func march(f32 u) -> Optional<Interaction> {
                return Interaction(
                    phase::Phase_Function_Union(),
                    r.o + t_max * r.d,
                    t_max, fv4(1.f),
                    {}, {}, {}, {}, {},
                );
            }
        }

        public func begin(math::Context ctx, f32 t_max) -> Iterator {
            return Iterator(ctx.r, t_max);
        }
    }
}
