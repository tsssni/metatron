implementing filter;

namespace mtt::filter {
    public struct Box_Filter: Filter {
        fv2 radius;

        public f32 operator()(fv2 p) {
            return all(abs(p) <= radius) ? 1.f : 0.f;
        }

        public func sample(fv2 u) -> Optional<Interaction> {
            let p = lerp(-radius, radius, u);
            let w = this(p);
            return Interaction(p, w, 1.f / (4.f * radius[0] * radius[1]));
        }
    }
}
